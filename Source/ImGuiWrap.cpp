#include "../Header/ImGuiWrap.h"

ImGuiWrap::ImGuiWrap(bool toggleSpin, bool toggleWireFrame, bool toggleGeometryShader,
    bool toggleGaussian, bool toggleTesselate, bool toggleEmissive) : 
    toggleSpin(toggleSpin), toggleWireFrame(toggleWireFrame), toggleGeometryShader(toggleGeometryShader),
    toggleGaussian(toggleGaussian), toggleTesselate(toggleTesselate), toggleEmissive(toggleEmissive)
{

}

ImGuiWrap::~ImGuiWrap()
{

}

void ImGuiWrap::SetupImGui(HWND hwnd, ID3D11Device* device_ptr, ID3D11DeviceContext* device_context_ptr)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device_ptr, device_context_ptr);
    ImGui::StyleColorsDark();
}

void ImGuiWrap::StartFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiWrap::RunFrame(float *tableRotation, float* tableScale, float* tableTranslation, 
    float* boxRotation, float* boxScale, float* boxTranslation,
    float& lightStrength, float& ambientStrength, float& spotAngle, 
    float* attenuationABC, float* lightProjection, float* tessFactor,
    float* emisIntens, float& emisPower)
{
    if (ImGui::Begin("Scene Controls"))
    {
        ImGui::DragFloat3("Room Rotation", tableRotation, 0.01f, 0.0f, 4.0f);
        ImGui::DragFloat3("Room Scale", tableScale, 0.01f, 0.1f, 1.0f);
        ImGui::DragFloat3("Room Translation", tableTranslation, 0.01f, -2.0f, 2.0f);
        ImGui::DragFloat3("Box Rotation", boxRotation, 0.01f, 0.0f, 4.0f);
        ImGui::DragFloat3("Box Scale", boxScale, 0.01f, 0.1f, 1.0f);
        ImGui::DragFloat3("Box Translation", boxTranslation, 0.01f, -2.0f, 2.0f);
        if (ImGui::Button("Box Spin"))
        {
            toggleSpin = !toggleSpin;
        }
        ImGui::DragFloat("Tess Factor X", &tessFactor[0], 1.0f, 1.0f, 3.0f);
        ImGui::DragFloat("Tess Factor Y", &tessFactor[1], 1.0f, 1.0f, 3.0f);
        ImGui::DragFloat("Tess Factor Z", &tessFactor[2], 1.0f, 1.0f, 3.0f);
        ImGui::DragFloat("Tess Factor W", &tessFactor[3], 0.001f, 0.0f, 0.1f);

    }
    ImGui::End();
    if (ImGui::Begin("Technique Toggles"))
    {
        if (ImGui::Button("Wire Frame"))
        {
            toggleWireFrame = !toggleWireFrame;
        }
        if (ImGui::Button("GS Toggle"))
        {
            toggleGeometryShader = !toggleGeometryShader;
        }
        if (ImGui::Button("Gaus Toggle"))
        {
            toggleGaussian = !toggleGaussian;
        }
        if (ImGui::Button("Tess Toggle"))
        {
            toggleTesselate = !toggleTesselate;
        }
        if (ImGui::Button("Emis Toggle"))
        {
            toggleEmissive = !toggleEmissive;
        }
    }
    ImGui::End();
    if (ImGui::Begin("Light Controls"))
    {
        ImGui::DragFloat("Light Strength", &lightStrength, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Ambient Strength", &ambientStrength, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Spotlight Diameter", &spotAngle, 0.01f, 1.0f, 45.0f);
        ImGui::DragFloat("Attenuation A", &attenuationABC[0], 0.01f, 0.1f, 10.0f);
        ImGui::DragFloat("Attenuation B", &attenuationABC[1], 0.01f, 0.0f, 10.0f);
        ImGui::DragFloat("Attenuation C", &attenuationABC[2], 0.01f, 0.0f, 10.0f);
        ImGui::DragFloat("Shadow Proj Width", &lightProjection[0], 0.01f, 1.0f, 10.0f);
        ImGui::DragFloat("Shadow Proj Height", &lightProjection[1], 0.01f, 1.0f, 10.0f);
        ImGui::DragFloat("Shadow Frustrum Near", &lightProjection[2], 0.01f, 0.01f, 10.0f);
        ImGui::DragFloat("Shadow Frustrum Far", &lightProjection[3], 0.01f, 1.0f, 20.0f);
        ImGui::DragFloat("Emiss R", &emisIntens[0], 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Emiss G", &emisIntens[1], 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Emiss B", &emisIntens[2], 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Emissive Power", &emisPower, 0.01f, 0.0f, 1.0f);
    }
    ImGui::End();

    ImGui::Render();
    // Render Draw Data
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiWrap::ShutDownImGui()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

bool& ImGuiWrap::GetToggleSpin()
{
	return this->toggleSpin;
}

bool& ImGuiWrap::GetToggleWF()
{
	return this->toggleWireFrame;
}

bool& ImGuiWrap::GetToggleGS()
{
	return this->toggleGeometryShader;
}

bool& ImGuiWrap::GetToggleGaus()
{
    return this->toggleGaussian;
}

bool& ImGuiWrap::GetToggleTesselate()
{
    return this->toggleTesselate;
}

bool& ImGuiWrap::GetToggleEmissive()
{
    return this->toggleEmissive;
}
