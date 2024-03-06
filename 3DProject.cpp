#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN    
#define STB_IMAGE_IMPLEMENTATION

#include <windows.h>
#include <d3dcompiler.h>

#pragma comment(lib, "user32")          // link against Win32 library
#pragma comment(lib, "d3d11.lib")       // direct3D library
#pragma comment(lib, "d3dcompiler.lib") // shader compiler

#include "Header/Object.h"
#include "Header/Texture.h"
#include "Header/Gbuffer.h"
#include "Header/Camera.h"
#include "Header/Light.h"
#include "Header/ShadowMap.h"
#include "Header/ImGuiWrap.h"
#include "Header/TimerWrap.h"

// Player Input
enum PlayerAction {
    PlayerActionMoveCamFwd,
    PlayerActionMoveCamBack,
    PlayerActionMoveCamLeft,
    PlayerActionMoveCamRight,
    PlayerActionTurnCamLeft,
    PlayerActionTurnCamRight,
    PlayerActionLookUp,
    PlayerActionLookDown,
    PlayerActionRaiseCam,
    PlayerActionLowerCam,
    PlayerActionPressL,
    PlayerActionCount
};
static bool global_keyIsDown[PlayerActionCount] = {};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT CreateShaders(UINT& flags, ID3D11Device* device_ptr, ID3DBlob*& vsgpass_blob_ptr, ID3DBlob*& error_blob, ID3D11VertexShader*& vertex_shader_ptr_gpass,
    ID3D11VertexShader*& vertex_shader_ptr_lpass, ID3D11PixelShader*& pixel_shader_ptr_gpass, ID3D11PixelShader*& pixel_shader_ptr_lpass, ID3D11GeometryShader*& geometry_shader_ptr,
    ID3D11VertexShader*& shadow_shader_ptr, ID3D11ComputeShader*& xBlur_shader_ptr, ID3D11ComputeShader*& yBlur_shader_ptr, ID3D11HullShader*& hull_shader_ptr, ID3D11DomainShader*& domain_shader_ptr);
HRESULT CreateSamplerStates(ID3D11Device* device_ptr, ID3D11SamplerState*& samplerStateWrap, ID3D11SamplerState*& samplerStateBoarder);
HRESULT CreateConstBuff(UINT ByteWidth, ID3D11Device* Device_ptr, ID3D11Buffer*& Buffer);
void CompileShader(LPCWSTR shaderName, LPCSTR entryPoint, LPCSTR target, UINT flags, ID3DBlob*& ptr, ID3DBlob*& error);
void UpdateMovement(Camera& camera, Light& light);
void UpdateUVOffset(ID3D11DeviceContext* device_context_ptr, float* uvOffset, float dt, float animate, ID3D11Buffer*& uvOffBuffer);
void UpdateTessFactor(ID3D11DeviceContext* device_context_ptr, float* tessFactors, ID3D11Buffer*& tessFactorBuffer);
void UpdateEmissive(ID3D11DeviceContext* device_context_ptr, bool emisToggle, DirectX::XMFLOAT3 emisIntens, float& emisPower, ID3D11Buffer*& emisBuffer);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow) 
{
    TimerWrap timer;
    timer.InitTimer();

    ID3D11Device* device_ptr = NULL;
    ID3D11DeviceContext* device_context_ptr = NULL;
    IDXGISwapChain* swap_chain_ptr = NULL;
    ID3D11RenderTargetView* back_buffer_view_ptr = NULL;
    ID3D11ShaderResourceView* back_buffer_srv = NULL;
    ID3D11UnorderedAccessView* back_buffer_uav = NULL;
    ID3D11UnorderedAccessView* blur_uav = NULL;
    ID3D11ShaderResourceView* blur_srv = NULL;
    ID3D11UnorderedAccessView* nullUAV = NULL;
    ID3D11ShaderResourceView* nullSRV[] = { NULL, NULL, NULL, NULL };
    ID3D11RenderTargetView* nullRTV[] = { NULL, NULL, NULL, NULL };

    const int WINDOW_WIDTH = GetSystemMetrics(SM_CXSCREEN);
    const int WINDOW_HEIGHT = GetSystemMetrics(SM_CYSCREEN);
    const int GBUFFER_COUNT = 4;
    const int TEXTURE_COUNT = 3;
    const int centerScreenX = GetSystemMetrics(SM_CXSCREEN) / 2 - WINDOW_WIDTH / 2;
    const int centerScreenY = GetSystemMetrics(SM_CYSCREEN) / 2 - WINDOW_HEIGHT / 2;

    // Register the window class
    const wchar_t CLASS_NAME[] = L"Sample Window Class";
    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);


    // Create the window
    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"3D Project", WS_OVERLAPPEDWINDOW, 
        centerScreenX, // X Pos
        centerScreenY,  // Y Pos
        WINDOW_WIDTH,  // Widht
        WINDOW_HEIGHT,  // Height
        NULL, NULL, hInstance, NULL);
    if (hwnd == NULL)
    {
        return 0;
    }
    ShowWindow(hwnd, nCmdShow);

    D3D_FEATURE_LEVEL feature_level;
    UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#if defined(DEBUG) || defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#else
    flags = 0;
#endif;

    DXGI_SWAP_CHAIN_DESC swap_chain_descr = { 0 };
    swap_chain_descr.BufferDesc.RefreshRate.Numerator = 0;
    swap_chain_descr.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_descr.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_descr.SampleDesc.Count = 1;
    swap_chain_descr.SampleDesc.Quality = 0;
    swap_chain_descr.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_UNORDERED_ACCESS;
    swap_chain_descr.BufferCount = 1;
    swap_chain_descr.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swap_chain_descr.OutputWindow = hwnd;
    swap_chain_descr.Windowed = false;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, NULL, 0, 
        D3D11_SDK_VERSION, &swap_chain_descr, &swap_chain_ptr, &device_ptr, &feature_level, &device_context_ptr);
    assert(S_OK == hr && swap_chain_ptr && device_ptr && device_context_ptr);

    ID3D11Texture2D* frameBuffer = NULL;
    assert(SUCCEEDED(swap_chain_ptr->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frameBuffer)));
    assert(SUCCEEDED(device_ptr->CreateRenderTargetView(frameBuffer, NULL, &back_buffer_view_ptr)));
    assert(SUCCEEDED(device_ptr->CreateUnorderedAccessView(frameBuffer, NULL, &back_buffer_uav)));
    assert(SUCCEEDED(device_ptr->CreateShaderResourceView(frameBuffer, NULL, &back_buffer_srv)));

    ID3D11Texture2D* depthBuffer = NULL;
    D3D11_TEXTURE2D_DESC depthBufferDesc = {};
    frameBuffer->GetDesc(&depthBufferDesc);
    frameBuffer->Release();
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.Width = WINDOW_WIDTH;
    depthBufferDesc.Height = WINDOW_HEIGHT;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.SampleDesc.Count = 1;

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
    depthStencilViewDesc.Format = depthBufferDesc.Format;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    assert(SUCCEEDED(device_ptr->CreateTexture2D(&depthBufferDesc, NULL, &depthBuffer)));

    ID3D11DepthStencilView* depthStencilView = NULL;
    assert(SUCCEEDED(device_ptr->CreateDepthStencilView(depthBuffer, &depthStencilViewDesc, &depthStencilView)));

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

    ID3D11DepthStencilState* depthStencilState = NULL;
    assert(SUCCEEDED(device_ptr->CreateDepthStencilState(&depthStencilDesc, &depthStencilState)));
    depthBuffer->Release();

    ///////////// SET UP VARIABLES FOR GAUSSIAN BLUR PASS ///////////// 
    ID3D11Texture2D * blurTexture = NULL;
    D3D11_TEXTURE2D_DESC tDesc;
    ZeroMemory(&tDesc, sizeof(D3D11_TEXTURE2D_DESC));
    tDesc.Width = WINDOW_WIDTH;
    tDesc.Height = WINDOW_HEIGHT;
    tDesc.MipLevels = 1;
    tDesc.ArraySize = 1;
    tDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    tDesc.SampleDesc.Count = 1;
    tDesc.SampleDesc.Quality = 0;
    tDesc.Usage = D3D11_USAGE_DEFAULT;
    tDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    tDesc.CPUAccessFlags = 0;
    tDesc.MiscFlags = 0;

    device_ptr->CreateTexture2D(&tDesc, NULL, &blurTexture);
    assert(SUCCEEDED(device_ptr->CreateUnorderedAccessView(blurTexture, NULL, &blur_uav)));
    assert(SUCCEEDED(device_ptr->CreateShaderResourceView(blurTexture, NULL, &blur_srv)));
    blurTexture->Release();

    flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG;
#endif;

    ID3DBlob* vsgpass_blob_ptr = NULL, *error_blob = NULL;
    ID3D11VertexShader* vertex_shader_ptr_gpass = NULL, * vertex_shader_ptr_lpass = NULL;
    ID3D11PixelShader* pixel_shader_ptr_gpass = NULL, * pixel_shader_ptr_lpass = NULL;
    ID3D11GeometryShader* geometry_shader_ptr = NULL;
    ID3D11VertexShader* shadow_shader_ptr = NULL;
    ID3D11ComputeShader* xBlur_shader_ptr = NULL;
    ID3D11ComputeShader* yBlur_shader_ptr = NULL;
    ID3D11HullShader* hull_shader_ptr = NULL;
    ID3D11DomainShader* domain_shader_ptr = NULL;

    // INITIALIZE SHADERS
    assert(SUCCEEDED(CreateShaders(flags, device_ptr, vsgpass_blob_ptr, error_blob, vertex_shader_ptr_gpass, vertex_shader_ptr_lpass, 
        pixel_shader_ptr_gpass, pixel_shader_ptr_lpass, geometry_shader_ptr, shadow_shader_ptr, xBlur_shader_ptr, yBlur_shader_ptr, hull_shader_ptr, domain_shader_ptr)));
    
    // INPUT ASSEMBLER
    ID3D11InputLayout* input_layout_ptr = NULL;
    {
        D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
            {"POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            {"NOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            {"TEX", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            {"TAN", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        hr = device_ptr->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), 
            vsgpass_blob_ptr->GetBufferPointer(), vsgpass_blob_ptr->GetBufferSize(), &input_layout_ptr);
        assert(SUCCEEDED(hr));
        vsgpass_blob_ptr->Release();
    }
    
    /////////////////////////// RASTERIZER ///////////////////////////
    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID; 
    rasterizerDesc.CullMode = D3D11_CULL_NONE; 
    rasterizerDesc.DepthClipEnable = true;

    ID3D11RasterizerState* rasterizerState_Solid = NULL;
    hr = device_ptr->CreateRasterizerState(&rasterizerDesc, &rasterizerState_Solid);
    assert(SUCCEEDED(hr));

    rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
    ID3D11RasterizerState* rasterizerState_Wire = NULL;
    hr = device_ptr->CreateRasterizerState(&rasterizerDesc, &rasterizerState_Wire);
    assert(SUCCEEDED(hr));

    /////////////////////////// SAMPLERS ///////////////////////////
    ID3D11SamplerState* samplerStateWrap = NULL;
    ID3D11SamplerState* samplerStateBoarder = NULL;
    assert(SUCCEEDED(CreateSamplerStates(device_ptr, samplerStateWrap, samplerStateBoarder)));

    ID3D11SamplerState* samplers[2] = { samplerStateWrap, samplerStateBoarder };

    /////////////////////////// G-BUFFERS ///////////////////////////
    // READ SRView // WRITE RTView
    Gbuffer* gBuffers[GBUFFER_COUNT] = {};
    for (int i = 0; i < GBUFFER_COUNT; i++)
    {
        gBuffers[i] = new Gbuffer(WINDOW_WIDTH, WINDOW_HEIGHT);
        gBuffers[i]->MakeGbuffer(device_ptr);
        gBuffers[i]->CreateRenderTargetView(device_ptr);
        gBuffers[i]->CreateShaderResourceView(device_ptr);
    }

    /////////////////////////// CONSTANT BUFFERS ///////////////////////////
    ID3D11Buffer* cameraBuffer = NULL, * objectBuffer = NULL, * lightBuffer = NULL,
        * materialBuffer = NULL, * cameraPositionBuffer = NULL, * lightWVPBuffer = NULL,
        * tessFactorBuffer = NULL, * emisBuffer = NULL, * uvOffBuffer = NULL;

    assert(SUCCEEDED(CreateConstBuff(sizeof(CameraBuffer), device_ptr, cameraBuffer)));
    assert(SUCCEEDED(CreateConstBuff(sizeof(ObjectBuffer), device_ptr, objectBuffer)));
    assert(SUCCEEDED(CreateConstBuff(sizeof(LightBuffer), device_ptr, lightBuffer)));
    assert(SUCCEEDED(CreateConstBuff(sizeof(MaterialBuffer), device_ptr, materialBuffer)));
    assert(SUCCEEDED(CreateConstBuff(sizeof(CameraPositionBuffer), device_ptr, cameraPositionBuffer)));
    assert(SUCCEEDED(CreateConstBuff(sizeof(LightWVPBuffer), device_ptr, lightWVPBuffer)));
    assert(SUCCEEDED(CreateConstBuff(sizeof(tessFactor), device_ptr, tessFactorBuffer)));
    assert(SUCCEEDED(CreateConstBuff(sizeof(emissiveBuffer), device_ptr, emisBuffer)));
    assert(SUCCEEDED(CreateConstBuff(sizeof(uvOffsetBuffer), device_ptr, uvOffBuffer)));
    
    /////////////////////////// CREATE ALL VERTEX BUFFERS ///////////////////////////
    float screen_quad[] = { //3 POS, 3 NOR, 2 TEX, 4 TAN
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 2.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f,

        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 2.0f, 0.0f, 0.0f, 1.0f,
        1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 2.0f, 0.0f, 0.0f, 1.0f,
    };
    UINT vertex_stride = 12 * sizeof(float);
    UINT vertex_offset = 0;
    UINT vertex_count = sizeof(screen_quad) / vertex_stride;

    ID3D11Buffer* vertex_buffer_screen_quad_ptr = NULL;
    {
        D3D11_BUFFER_DESC vertex_buff_descr = {};
        vertex_buff_descr.ByteWidth = sizeof(screen_quad);
        vertex_buff_descr.StructureByteStride = sizeof(float);
        vertex_buff_descr.CPUAccessFlags = 0;
        vertex_buff_descr.Usage = D3D11_USAGE_DEFAULT;
        vertex_buff_descr.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertex_buff_descr.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA subresource_data = { screen_quad };
        assert(SUCCEEDED(device_ptr->CreateBuffer(&vertex_buff_descr, &subresource_data, &vertex_buffer_screen_quad_ptr)));
    }

    /////////////////////////// LOAD ALL OBJECTS ///////////////////////////
    Object table;
    table.LoadObj("Assets/ClothTableInOnePart.obj");
    assert(SUCCEEDED(table.createVertexBuffer(device_ptr)));

    Object box;
    box.LoadObj("Assets/simple_box2.obj");
    assert(SUCCEEDED(box.createVertexBuffer(device_ptr)));

    Object roomObject;
    roomObject.LoadObj("Assets/brick_and_wood_room.obj");
    assert(SUCCEEDED(roomObject.createVertexBuffer(device_ptr)));

    Object glowBox;
    glowBox.LoadObj("Assets/simple_box3.obj");
    assert(SUCCEEDED(glowBox.createVertexBuffer(device_ptr)));

    Object waterBall;
    waterBall.LoadObj("Assets/simpleBall.obj");
    assert(SUCCEEDED(waterBall.createVertexBuffer(device_ptr)));

    /////////////////////////// TEXTURES FOR GEOMETRY PIXEL SHADER ///////////////////////////
    table.createTexture(device_ptr);
    box.createTexture(device_ptr);
    roomObject.createTexture(device_ptr);
    glowBox.createTexture(device_ptr);
    waterBall.createTexture(device_ptr);

    table.UpdateMaterialBuffer(device_context_ptr, materialBuffer);
    waterBall.setRST(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.3f, 0.3f, 0.3f), DirectX::XMFLOAT3(-1.0f, 1.0f, 0.5f));
    
    Texture emisMap;
    emisMap.LoadTextureFromName("Assets/bowie_emissive.png", device_ptr);
    ID3D11ShaderResourceView* emissiveMap = NULL;
    emisMap.getSRView(device_ptr, emissiveMap);

    Texture dispMap;
    dispMap.LoadTextureFromName("Assets/Water_001_DISP.png", device_ptr);
    ID3D11ShaderResourceView* displacementMap = NULL;
    dispMap.getSRView(device_ptr, displacementMap);
    
    /////////////////////////// ARRAYS OF CONSTANT BUFFERS AND GBUFFERS ///////////////////////////
    ID3D11RenderTargetView* gPassRTViews[GBUFFER_COUNT] = { gBuffers[0]->getRTView(), gBuffers[1]->getRTView(), gBuffers[2]->getRTView(), gBuffers[3]->getRTView() };
    ID3D11ShaderResourceView* lPassSRViews[GBUFFER_COUNT] = { gBuffers[0]->getSRView(), gBuffers[1]->getSRView(), gBuffers[2]->getSRView(), gBuffers[3]->getSRView() };
    ID3D11Buffer* lightBufferArr[] = { lightBuffer, lightWVPBuffer };
    ID3D11Buffer* gPassVSCbuffers[] = { objectBuffer, cameraBuffer, lightWVPBuffer };
    ID3D11Buffer* gPassCbuffers[] = { objectBuffer, cameraBuffer };
    ID3D11Buffer* lPassCbuffers[] = { lightBuffer, materialBuffer };

    /////////////////////////// FLOATS FOR CONSTANT BUFFERS AND IMGUI ///////////////////////////
    float lightStrength   = 0.5f;
    float ambientStrength = 0.2f;   
    float emisPower       = 1.0f;
    float spotAngle       = 10.0f;
    static float tableRotation[3]    = { 0.0f, 0.0f, 0.0f };
    static float tableScale[3]       = { 0.1f, 0.1f, 0.1f };
    static float tableTranslation[3] = { 0.0f, 0.0f, 0.0f };
    static float boxRotation[3]      = { 0.0f, 0.0f, 0.0f };
    static float boxScale[3]         = { 0.1f, 0.1f, 0.1f };
    static float boxTranslation[3]   = { 0.3f, 0.5f, 0.0f };
    static float attenuationABC[3]   = { 0.1f, 0.1f, 0.1f };
    static float lightProjection[4]  = { 5.0f, 5.0f, 0.01f, 10.0f };
    static float tessFactors[4]      = { 1.0f, 1.0f, 1.0f, 0.01f };
    static float emisIntens[3]       = { 1.0f, 1.0f, 1.0f };
    static float uvOffset[2] = { 0.0f, 0.0f };
    float dt = 0;

    /////////////////////////// VIEWPORT AND COLORS TO CLEAR BACKBUFFER AND GBUFFERS ///////////////////////////
    float background_color[4] = { 0x64 / 255.0f, 0x95 / 255.0f, 0xED / 255.0f, 1.0f };
    float black_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    D3D11_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(depthBufferDesc.Width), static_cast<float>(depthBufferDesc.Height), 0.0f, 1.0f };
    /////////////////////////// CAMERA, LIGHT AND SHADOW MAP ///////////////////////////
    Camera camera;
    camera.SetPosition(0.0f, 0.5f, 1.0f);
    camera.SetProjectionInput(90.0f, static_cast<float>(depthBufferDesc.Width) / static_cast<float>(depthBufferDesc.Height), 0.1f, 100.0f);
    camera.SetLookAtPos(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
    Light light;
    ShadowMap shadow(device_ptr, depthBufferDesc.Width, depthBufferDesc.Height);

    /////////////////////////// SETUP IMGUI ///////////////////////////
    ImGuiWrap imGuiWrap;
    imGuiWrap.SetupImGui(hwnd, device_ptr, device_context_ptr);

    /////////////////////////// RUN THE MESSAGE LOOP ///////////////////////////
    MSG msg = { };
    bool should_close = false;
    while (!should_close) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (msg.message == WM_QUIT) { break; }

        {
            if (imGuiWrap.GetToggleSpin())
                timer.UpdateTimeInterval();

            dt = timer.GetTimeInterval();
            // UPDATE BUFFERS FOR TESSELATION AND EMISSION // 
            UpdateTessFactor(device_context_ptr, tessFactors, tessFactorBuffer);
            UpdateUVOffset(device_context_ptr, uvOffset, dt, 0.0f, uvOffBuffer);
            UpdateEmissive(device_context_ptr, imGuiWrap.GetToggleEmissive(),
                DirectX::XMFLOAT3(emisIntens[0], emisIntens[1], emisIntens[2]), emisPower, emisBuffer);

            // UPDATE ROTATION, SCALE AND TRANSLATION MATRICIES FOR OBJECTS // 
            table.setRST(DirectX::XMFLOAT3(tableRotation[0], tableRotation[1], tableRotation[2]), 
                DirectX::XMFLOAT3(tableScale[0], tableScale[1], tableScale[2]), DirectX::XMFLOAT3(tableTranslation[0], tableTranslation[1], tableTranslation[2]));
            box.setRST(DirectX::XMFLOAT3(boxRotation[0] + dt, boxRotation[1] + dt, boxRotation[2] + dt),
                DirectX::XMFLOAT3(boxScale[0], boxScale[1], boxScale[2]), DirectX::XMFLOAT3(boxTranslation[0], boxTranslation[1], boxTranslation[2]));
            glowBox.setRST(DirectX::XMFLOAT3(boxRotation[0], boxRotation[1], boxRotation[2]),
                DirectX::XMFLOAT3(boxScale[0], boxScale[1], boxScale[2]), DirectX::XMFLOAT3(boxTranslation[0] - 1.0f, boxTranslation[1], boxTranslation[2]));
            roomObject.setRST(DirectX::XMFLOAT3(tableRotation[0], tableRotation[1], tableRotation[2]), 
                DirectX::XMFLOAT3(tableScale[0], tableScale[1], tableScale[2]), DirectX::XMFLOAT3(tableTranslation[0], tableTranslation[1], tableTranslation[2]));

            // UPDATE PLAYER CAMERA AND LIGHT VARIABLES // 
            light.SetProjectionInput(lightProjection[0], lightProjection[1], lightProjection[2], lightProjection[3]);
            UpdateMovement(camera, light);
            light.SetLightStrength(lightStrength);
            light.SetAmbientStrength(ambientStrength);
            light.SetAttenuationABC(attenuationABC[0], attenuationABC[1], attenuationABC[2]);
            light.SetSpotAngle(spotAngle);

            /*-------------------------SHADOW MAPPING-------------------------*/
            device_context_ptr->IASetInputLayout(input_layout_ptr);
            device_context_ptr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            if (imGuiWrap.GetToggleWF())
                device_context_ptr->RSSetState(rasterizerState_Wire);
            else
                device_context_ptr->RSSetState(rasterizerState_Solid);

            shadow.BindTargets(device_context_ptr, shadow_shader_ptr);

            light.UpdateLight(device_context_ptr, lightBufferArr, table);
            device_context_ptr->VSSetConstantBuffers(0, 1, &lightBufferArr[1]);
            device_context_ptr->IASetVertexBuffers(0, 1, &table.getVertexBuffer(), &table.vertex_stride, &vertex_offset);
            device_context_ptr->Draw(table.vertex_count, 0);

            light.UpdateLight(device_context_ptr, lightBufferArr, box);
            device_context_ptr->IASetVertexBuffers(0, 1, &box.getVertexBuffer(), &box.vertex_stride, &vertex_offset);
            device_context_ptr->Draw(box.vertex_count, 0);

            table.setShadowDepthMap(shadow.DepthMapSRV());
            box.setShadowDepthMap(shadow.DepthMapSRV());
            roomObject.setShadowDepthMap(shadow.DepthMapSRV());
            glowBox.setShadowDepthMap(shadow.DepthMapSRV());
            waterBall.setShadowDepthMap(shadow.DepthMapSRV());

            /*-------------------------GEOMETRY PASS-------------------------*/
            // SET VIEWPORT //
            device_context_ptr->ClearRenderTargetView(back_buffer_view_ptr, background_color);
            device_context_ptr->RSSetViewports(1, &viewport);
            device_context_ptr->OMSetDepthStencilState(depthStencilState, 0);
            device_context_ptr->OMSetRenderTargets(GBUFFER_COUNT, gPassRTViews, depthStencilView);
          
            // CLEAR GBUFFERS //
            for(int i = 0; i < GBUFFER_COUNT; i++)
                device_context_ptr->ClearRenderTargetView(gPassRTViews[i], black_color); // BLACK_COLOR HAS ALPHA 0 FOR A CHECK IN PS_LPASS RELATED TO GLOW MAPPING 
            device_context_ptr->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

            // SET SHADERS //
            device_context_ptr->VSSetShader(vertex_shader_ptr_gpass, NULL, 0);
            if (imGuiWrap.GetToggleGS())
                device_context_ptr->GSSetShader(geometry_shader_ptr, NULL, 0);
            else
                device_context_ptr->GSSetShader(NULL, NULL, 0);

            device_context_ptr->HSSetShader(NULL, NULL, 0);
            device_context_ptr->DSSetShader(NULL, NULL, 0);
            device_context_ptr->PSSetShader(pixel_shader_ptr_gpass, NULL, 0);
      
            device_context_ptr->PSSetSamplers(0, 2, samplers);
          
            // SET ALL CONSTANT BUFFERS AND DRAW //
            device_context_ptr->VSSetConstantBuffers(0, 3, gPassVSCbuffers);
            device_context_ptr->GSSetConstantBuffers(0, 1, &cameraPositionBuffer);
            device_context_ptr->PSSetConstantBuffers(0, 1, &lightBuffer);
            device_context_ptr->PSSetConstantBuffers(1, 1, &emisBuffer);
            device_context_ptr->PSSetConstantBuffers(2, 1, &uvOffBuffer);
            camera.UpdatePosBuff(device_context_ptr, cameraPositionBuffer);

            // SET NEW PS RESOURCES AND NEW VERTEX BUFFERS FOR EVERY OBJECT SINCE THEY HAVE THEIR OWN TRANSFORMS //
            camera.UpdateCBuffers(device_context_ptr, gPassCbuffers, table);
            table.draw(device_context_ptr, vertex_offset);

            camera.UpdateCBuffers(device_context_ptr, gPassCbuffers, box);
            box.draw(device_context_ptr, vertex_offset);

            camera.UpdateCBuffers(device_context_ptr, gPassCbuffers, roomObject);
            roomObject.draw(device_context_ptr, vertex_offset);

            camera.UpdateCBuffers(device_context_ptr, gPassCbuffers, glowBox);
            if (!imGuiWrap.GetToggleEmissive())
                glowBox.draw(device_context_ptr, vertex_offset);
            else
                glowBox.drawEmissive(device_context_ptr, vertex_offset, emissiveMap);
                

            device_context_ptr->PSSetShaderResources(3, 1, &nullSRV[0]);
            UpdateUVOffset(device_context_ptr, uvOffset, dt, 1.0f, uvOffBuffer);
           
            if (imGuiWrap.GetToggleTesselate())
            {
                device_context_ptr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
                device_context_ptr->HSSetShader(hull_shader_ptr, NULL, 0);
                device_context_ptr->DSSetShader(domain_shader_ptr, NULL, 0);
                device_context_ptr->HSSetConstantBuffers(0, 1, &tessFactorBuffer);
                device_context_ptr->DSSetConstantBuffers(0, 1, &gPassCbuffers[1]);
                device_context_ptr->DSSetConstantBuffers(1, 1, &tessFactorBuffer);
                device_context_ptr->DSSetConstantBuffers(2, 1, &uvOffBuffer);
                device_context_ptr->DSSetSamplers(0, 1, &samplers[0]);
                device_context_ptr->DSSetShaderResources(0, 1, &displacementMap);
            }
            
            camera.UpdateCBuffers(device_context_ptr, gPassCbuffers, waterBall);
            waterBall.draw(device_context_ptr, vertex_offset);

            /*-------------------------LIGHT PASS-------------------------*/
            device_context_ptr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            if (imGuiWrap.GetToggleWF())
                device_context_ptr->RSSetState(rasterizerState_Solid);

            device_context_ptr->ClearRenderTargetView(back_buffer_view_ptr, background_color);
            device_context_ptr->OMSetRenderTargets(GBUFFER_COUNT, nullRTV, NULL);
            device_context_ptr->PSSetShaderResources(0, GBUFFER_COUNT, nullSRV);

            device_context_ptr->OMSetRenderTargets(1, &back_buffer_view_ptr, NULL);
            device_context_ptr->IASetVertexBuffers(0, 1, &vertex_buffer_screen_quad_ptr, &vertex_stride, &vertex_offset);
            device_context_ptr->VSSetShader(vertex_shader_ptr_lpass, NULL, 0);
            device_context_ptr->GSSetShader(NULL, NULL, 0);
            device_context_ptr->HSSetShader(NULL, NULL, 0);
            device_context_ptr->DSSetShader(NULL, NULL, 0);
            device_context_ptr->PSSetShader(pixel_shader_ptr_lpass, NULL, 0);
            device_context_ptr->PSSetShaderResources(0, GBUFFER_COUNT, lPassSRViews);
            device_context_ptr->PSSetSamplers(0, 1, &samplers[0]);
            device_context_ptr->PSSetConstantBuffers(0, 2, lPassCbuffers);
            device_context_ptr->Draw(vertex_count, 0);

            // SEND TRANSFORMATION VARIABLES TO IMGUI
            imGuiWrap.StartFrame();
            imGuiWrap.RunFrame(tableRotation, tableScale, tableTranslation, boxRotation, boxScale, boxTranslation,
                lightStrength, ambientStrength, spotAngle, attenuationABC, lightProjection, tessFactors, emisIntens, emisPower);
            
            device_context_ptr->OMSetRenderTargets(GBUFFER_COUNT, nullRTV, NULL);
            device_context_ptr->PSSetShaderResources(0, GBUFFER_COUNT, nullSRV);
            device_context_ptr->DSSetShaderResources(0, 1, &nullSRV[0]);

            /*-------------------------GAUSSIAN BLUR PASS-------------------------*/
            if (imGuiWrap.GetToggleGaus())
            {
                device_context_ptr->CSSetShader(xBlur_shader_ptr, NULL, 0);
                device_context_ptr->CSSetShaderResources(0, 1, &back_buffer_srv);
                device_context_ptr->CSSetUnorderedAccessViews(0, 1, &blur_uav, NULL);
                device_context_ptr->Dispatch(WINDOW_WIDTH / 16, WINDOW_HEIGHT / 16, 1);
                device_context_ptr->CSSetShaderResources(0, 1, &nullSRV[0]);
                device_context_ptr->CSSetUnorderedAccessViews(0, 1, &nullUAV, NULL);
 
                device_context_ptr->CSSetShader(yBlur_shader_ptr, NULL, 0);
                device_context_ptr->CSSetShaderResources(0, 1, &blur_srv);
                device_context_ptr->CSSetUnorderedAccessViews(0, 1, &back_buffer_uav, NULL);
                device_context_ptr->Dispatch(WINDOW_WIDTH / 16, WINDOW_HEIGHT / 16, 1);
                device_context_ptr->CSSetShaderResources(0, 1, &nullSRV[0]);
                device_context_ptr->CSSetUnorderedAccessViews(0, 1, &nullUAV, NULL);
            }
            swap_chain_ptr->Present(1, 0);
        }
    }
    imGuiWrap.ShutDownImGui();


    for(int i = 0; i < GBUFFER_COUNT; i++)
        gBuffers[i]->~Gbuffer();

    table.release();
    box.release();
    roomObject.release();
    glowBox.release();
    waterBall.release();
    
    back_buffer_view_ptr->Release();
    back_buffer_uav->Release();
    back_buffer_srv->Release();
    blur_uav->Release();
    blur_srv->Release();
    depthStencilView->Release();
    depthStencilState->Release();
    vertex_shader_ptr_gpass->Release();
    vertex_shader_ptr_lpass->Release();
    pixel_shader_ptr_gpass->Release();
    pixel_shader_ptr_lpass->Release();
    geometry_shader_ptr->Release();
    shadow_shader_ptr->Release();
    xBlur_shader_ptr->Release();
    yBlur_shader_ptr->Release();
    hull_shader_ptr->Release();
    domain_shader_ptr->Release();
    input_layout_ptr->Release();
    rasterizerState_Solid->Release();
    rasterizerState_Wire->Release();
    samplerStateWrap->Release();
    samplerStateBoarder->Release();
    vertex_buffer_screen_quad_ptr->Release();
    cameraBuffer->Release();
    objectBuffer->Release();
    lightBuffer->Release();
    materialBuffer->Release();
    cameraPositionBuffer->Release();
    lightWVPBuffer->Release();
    tessFactorBuffer->Release();
    emisBuffer->Release();
    uvOffBuffer->Release();
    displacementMap->Release();
    emissiveMap->Release();
    device_ptr->Release();
    device_context_ptr->Release();
    swap_chain_ptr->Release();
    device_context_ptr->ClearState();
    device_context_ptr->Flush();
    
    return 0;
}   

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
    {
        return true;
    }
    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
        bool isDown = (uMsg == WM_KEYDOWN);
        switch (wParam) {
            case VK_ESCAPE:
                DestroyWindow(hwnd);
                break;
            case 'W':
                global_keyIsDown[PlayerActionMoveCamFwd] = isDown;
                break;
            case 'A':
                global_keyIsDown[PlayerActionMoveCamLeft] = isDown;
                break;
            case 'S':
                global_keyIsDown[PlayerActionMoveCamBack] = isDown;
                break;
            case 'D':
                global_keyIsDown[PlayerActionMoveCamRight] = isDown;
                break;
            case 'E':
                global_keyIsDown[PlayerActionRaiseCam] = isDown;
                break;
            case 'Q':
                global_keyIsDown[PlayerActionLowerCam] = isDown;
                break;
            case VK_UP:
                global_keyIsDown[PlayerActionLookUp] = isDown;
                break;
            case VK_LEFT:
                global_keyIsDown[PlayerActionTurnCamLeft] = isDown;
                break;
            case VK_DOWN:
                global_keyIsDown[PlayerActionLookDown] = isDown;
                break;
            case VK_RIGHT:
                global_keyIsDown[PlayerActionTurnCamRight] = isDown;
                break;
            case 'L':
                global_keyIsDown[PlayerActionPressL] = isDown;
                break;
        }
        break;
    }

    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }

    case WM_CLOSE:
        if (MessageBox(hwnd, L"Really quit?", L"3D Project", MB_OKCANCEL) == IDOK)
        {
            DestroyWindow(hwnd);
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HRESULT CreateShaders(UINT& flags, ID3D11Device* device_ptr, ID3DBlob*& vsgpass_blob_ptr, ID3DBlob*& error_blob, ID3D11VertexShader*& vertex_shader_ptr_gpass,
    ID3D11VertexShader*& vertex_shader_ptr_lpass, ID3D11PixelShader*& pixel_shader_ptr_gpass, ID3D11PixelShader*& pixel_shader_ptr_lpass, ID3D11GeometryShader*& geometry_shader_ptr,
    ID3D11VertexShader*& shadow_shader_ptr, ID3D11ComputeShader*& xBlur_shader_ptr, ID3D11ComputeShader*& yBlur_shader_ptr, ID3D11HullShader*& hull_shader_ptr, ID3D11DomainShader*& domain_shader_ptr)
{
    ID3DBlob* vslpass_blob_ptr = NULL, * psgpass_blob_ptr = NULL, * pslpass_blob_ptr = NULL,
        * geometry_blob_ptr = NULL, * shadow_blob_ptr = NULL, * xBlur_blob_ptr = NULL, * yBlur_blob_ptr = NULL,
        * hull_blob_ptr = NULL, * domain_blob_ptr = NULL;

    CompileShader(L"Shaders/VS_GPASS.hlsl", "vs_main", "vs_5_0", flags, vsgpass_blob_ptr, error_blob);
    CompileShader(L"Shaders/VS_LPASS.hlsl", "vs_main", "vs_5_0", flags, vslpass_blob_ptr, error_blob);
    CompileShader(L"Shaders/PS_GPASS.hlsl", "ps_main", "ps_5_0", flags, psgpass_blob_ptr, error_blob);
    CompileShader(L"Shaders/PS_LPASS.hlsl", "ps_main", "ps_5_0", flags, pslpass_blob_ptr, error_blob);
    CompileShader(L"Shaders/GeometryShader.hlsl", "main", "gs_5_0", flags, geometry_blob_ptr, error_blob);
    CompileShader(L"Shaders/VS_SHADOW.hlsl", "shadow_main", "vs_5_0", flags, shadow_blob_ptr, error_blob);
    CompileShader(L"Shaders/xBlurCS.hlsl", "main", "cs_5_0", flags, xBlur_blob_ptr, error_blob);
    CompileShader(L"Shaders/yBlurCS.hlsl", "main", "cs_5_0", flags, yBlur_blob_ptr, error_blob);
    CompileShader(L"Shaders/HullShader.hlsl", "PerPatchFunction", "hs_5_0", flags, hull_blob_ptr, error_blob);
    CompileShader(L"Shaders/DomainShader.hlsl", "main", "ds_5_0", flags, domain_blob_ptr, error_blob);

    // CREATE SHADERS //
    HRESULT hr;
    {
        hr = device_ptr->CreateVertexShader(vsgpass_blob_ptr->GetBufferPointer(), vsgpass_blob_ptr->GetBufferSize(), NULL, &vertex_shader_ptr_gpass);
        assert(SUCCEEDED(hr));
        hr = device_ptr->CreateVertexShader(vslpass_blob_ptr->GetBufferPointer(), vslpass_blob_ptr->GetBufferSize(), NULL, &vertex_shader_ptr_lpass);
        assert(SUCCEEDED(hr));
        hr = device_ptr->CreatePixelShader(psgpass_blob_ptr->GetBufferPointer(), psgpass_blob_ptr->GetBufferSize(), NULL, &pixel_shader_ptr_gpass);
        assert(SUCCEEDED(hr));
        hr = device_ptr->CreatePixelShader(pslpass_blob_ptr->GetBufferPointer(), pslpass_blob_ptr->GetBufferSize(), NULL, &pixel_shader_ptr_lpass);
        assert(SUCCEEDED(hr));
        hr = device_ptr->CreateGeometryShader(geometry_blob_ptr->GetBufferPointer(), geometry_blob_ptr->GetBufferSize(), NULL, &geometry_shader_ptr);
        assert(SUCCEEDED(hr));
        hr = device_ptr->CreateVertexShader(shadow_blob_ptr->GetBufferPointer(), shadow_blob_ptr->GetBufferSize(), NULL, &shadow_shader_ptr);
        assert(SUCCEEDED(hr));
        hr = device_ptr->CreateComputeShader(xBlur_blob_ptr->GetBufferPointer(), xBlur_blob_ptr->GetBufferSize(), NULL, &xBlur_shader_ptr);
        assert(SUCCEEDED(hr));
        hr = device_ptr->CreateComputeShader(yBlur_blob_ptr->GetBufferPointer(), yBlur_blob_ptr->GetBufferSize(), NULL, &yBlur_shader_ptr);
        assert(SUCCEEDED(hr));
        hr = device_ptr->CreateHullShader(hull_blob_ptr->GetBufferPointer(), hull_blob_ptr->GetBufferSize(), NULL, &hull_shader_ptr);
        assert(SUCCEEDED(hr));
        hr = device_ptr->CreateDomainShader(domain_blob_ptr->GetBufferPointer(), domain_blob_ptr->GetBufferSize(), NULL, &domain_shader_ptr);
        assert(SUCCEEDED(hr));

        vslpass_blob_ptr->Release();
        psgpass_blob_ptr->Release();
        pslpass_blob_ptr->Release();
        geometry_blob_ptr->Release();
        shadow_blob_ptr->Release();
        xBlur_blob_ptr->Release();
        yBlur_blob_ptr->Release();
        hull_blob_ptr->Release();
        domain_blob_ptr->Release();
    }
    return hr;
}

HRESULT CreateSamplerStates(ID3D11Device* device_ptr, ID3D11SamplerState*& samplerStateWrap, ID3D11SamplerState*& samplerStateBoarder)
{
    D3D11_SAMPLER_DESC samplerDescWrap = {};
    samplerDescWrap.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDescWrap.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDescWrap.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDescWrap.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDescWrap.MinLOD = 0.0f;
    samplerDescWrap.MaxLOD = D3D11_FLOAT32_MAX;
    samplerDescWrap.MipLODBias = 0.0f;
    samplerDescWrap.MaxAnisotropy = 0;
    samplerDescWrap.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

    HRESULT hr = device_ptr->CreateSamplerState(&samplerDescWrap, &samplerStateWrap);
    assert(SUCCEEDED(hr));

    D3D11_SAMPLER_DESC samplerDescBoarder = {};
    samplerDescBoarder.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDescBoarder.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDescBoarder.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDescBoarder.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDescBoarder.BorderColor[0] = 1.0f;
    samplerDescBoarder.BorderColor[1] = 1.0f;
    samplerDescBoarder.BorderColor[2] = 1.0f;
    samplerDescBoarder.BorderColor[3] = 1.0f;
    samplerDescBoarder.MinLOD = 0.0f;
    samplerDescBoarder.MaxLOD = D3D11_FLOAT32_MAX;
    samplerDescBoarder.MipLODBias = 0.0f;
    samplerDescBoarder.MaxAnisotropy = 0;
    samplerDescBoarder.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

    hr = device_ptr->CreateSamplerState(&samplerDescBoarder, &samplerStateBoarder);
    assert(SUCCEEDED(hr));

    return hr;
}

HRESULT CreateConstBuff(UINT ByteWidth, ID3D11Device* Device_ptr, ID3D11Buffer*& Buffer)
{
    D3D11_BUFFER_DESC constantBufferDesc = {};
    constantBufferDesc.ByteWidth = static_cast<UINT>(ByteWidth + (16 - (ByteWidth % 16)));
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = Device_ptr->CreateBuffer(&constantBufferDesc, nullptr, &Buffer);
    return hr;
}

void CompileShader(LPCWSTR shaderName, LPCSTR entryPoint, LPCSTR target, UINT flags, ID3DBlob*& ptr, ID3DBlob*& error)
{
    HRESULT hr = D3DCompileFromFile(shaderName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, target, flags, 0, &ptr, &error);
    if (FAILED(hr)) {
        if (error) {
            OutputDebugStringA((char*)error->GetBufferPointer());
            error->Release();
        }
        if (ptr) { ptr->Release(); }
        assert(false);
    }
}

void UpdateMovement(Camera& camera, Light& light)
{
    // WASD, UP AND DOWN //
    DirectX::XMFLOAT3 cameraSpeedFloat = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
    DirectX::XMVECTOR cameraSpeedVec = DirectX::XMLoadFloat3(&cameraSpeedFloat);
    if (global_keyIsDown[PlayerActionMoveCamFwd])
    {
        camera.AdjustPosition(DirectX::XMVectorMultiply(camera.GetForwardVector(), cameraSpeedVec));
    }
    if (global_keyIsDown[PlayerActionMoveCamBack])
    {
        camera.AdjustPosition(DirectX::XMVectorMultiply(camera.GetBackwardVector(), cameraSpeedVec));
    }
    if (global_keyIsDown[PlayerActionMoveCamLeft])
    {
        camera.AdjustPosition(DirectX::XMVectorMultiply(camera.GetLeftVector(), cameraSpeedVec));
    }
    if (global_keyIsDown[PlayerActionMoveCamRight])
    {
        camera.AdjustPosition(DirectX::XMVectorMultiply(camera.GetRightVector(), cameraSpeedVec));
    }
    if (global_keyIsDown[PlayerActionRaiseCam])
    {
        camera.AdjustPosition(0.0f, cameraSpeedFloat.y, 0.0f);
    }
    if (global_keyIsDown[PlayerActionLowerCam])
    {
        camera.AdjustPosition(0.0f, -cameraSpeedFloat.y, 0.0f);
    }

    // ARROW KEYS //
    DirectX::XMFLOAT3 cameraRotationFloat;
    DirectX::XMVECTOR cameraRotationVec;
    if (global_keyIsDown[PlayerActionTurnCamLeft])
    {
        cameraRotationFloat = DirectX::XMFLOAT3(0.0f, -0.03f, 0.0f);
        cameraRotationVec = DirectX::XMLoadFloat3(&cameraRotationFloat);
        camera.AdjustRotation(cameraRotationVec);
    }
    if (global_keyIsDown[PlayerActionTurnCamRight])
    {
        cameraRotationFloat = DirectX::XMFLOAT3(0.0f, 0.03f, 0.0f);
        cameraRotationVec = DirectX::XMLoadFloat3(&cameraRotationFloat);
        camera.AdjustRotation(cameraRotationVec);
    }
    if (global_keyIsDown[PlayerActionLookUp])
    {
        cameraRotationFloat = DirectX::XMFLOAT3(-0.03f, 0.0f, 0.0f);
        cameraRotationVec = DirectX::XMLoadFloat3(&cameraRotationFloat);
        camera.AdjustRotation(cameraRotationVec);
    }
    if (global_keyIsDown[PlayerActionLookDown])
    {
        cameraRotationFloat = DirectX::XMFLOAT3(0.03f, 0.0f, 0.0f);
        cameraRotationVec = DirectX::XMLoadFloat3(&cameraRotationFloat);
        camera.AdjustRotation(cameraRotationVec);
    }

    // MISC BUTTONS //
    if (global_keyIsDown[PlayerActionPressL])
    {
        DirectX::XMVECTOR lightPos = camera.GetPositionVector();
        DirectX::XMVectorAdd(lightPos, camera.GetForwardVector());
        light.SetPosition(lightPos);
        light.SetRotation(camera.GetRotationFloat3());
        light.SetLightDirection(camera.GetForwardVector());
    }
}

void UpdateUVOffset(ID3D11DeviceContext* device_context_ptr, float* uvOffset, float dt, float animate, ID3D11Buffer*& uvOffBuffer)
{
    uvOffset[0] = 0.1f * dt;
    uvOffset[1] = 0.05f * dt;
    uvOffsetBuffer Ldata;
    Ldata.uvOffsets = DirectX::XMFLOAT2(uvOffset[0], uvOffset[1]);
    Ldata.animate = animate;
    

    D3D11_MAPPED_SUBRESOURCE uvMap;
    device_context_ptr->Map(uvOffBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &uvMap);
    CopyMemory(uvMap.pData, &Ldata, sizeof(uvOffsetBuffer));
    device_context_ptr->Unmap(uvOffBuffer, 0);
}

void UpdateTessFactor(ID3D11DeviceContext* device_context_ptr, float* tessFactors, ID3D11Buffer*& tessFactorBuffer)
{
    tessFactor Ldata;
    Ldata.tessFx = tessFactors[0];
    Ldata.tessFy = tessFactors[1];
    Ldata.tessFz = tessFactors[2];
    Ldata.tessFw = tessFactors[3];
    D3D11_MAPPED_SUBRESOURCE tessMap;
    device_context_ptr->Map(tessFactorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &tessMap);
    CopyMemory(tessMap.pData, &Ldata, sizeof(tessFactor));
    device_context_ptr->Unmap(tessFactorBuffer, 0);
}

void UpdateEmissive(ID3D11DeviceContext* device_context_ptr, bool emisToggle, DirectX::XMFLOAT3 emisIntens, float& emisPower, ID3D11Buffer*& emisBuffer)
{
    emissiveBuffer Ldata;
    Ldata.isEmissive = emisToggle;
    Ldata.emissiveIntensity = emisIntens;
    Ldata.emissivePower = emisPower;

    D3D11_MAPPED_SUBRESOURCE emisMap;
    device_context_ptr->Map(emisBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &emisMap);
    CopyMemory(emisMap.pData, &Ldata, sizeof(emissiveBuffer));
    device_context_ptr->Unmap(emisBuffer, 0);
}