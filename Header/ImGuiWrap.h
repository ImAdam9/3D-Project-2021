#ifndef IMGUIWRAP_H
#define IMGUIWRAP_H
#include <d3d11.h>
#include "../imgui.h"
#include "../imgui_impl_dx11.h"
#include "../imgui_impl_win32.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
class ImGuiWrap
{
private:
	bool toggleSpin;
	bool toggleWireFrame;
	bool toggleGeometryShader;
	bool toggleGaussian;
	bool toggleTesselate;
	bool toggleEmissive;
public:
	ImGuiWrap(bool toggleSpin = false, bool toggleWireFrame = false, bool toggleGeometryShader = false,
		bool toggleGaussian = false, bool toggleTesselate = false, bool toggleEmissive = false);
	~ImGuiWrap();

	void SetupImGui(HWND hwnd, ID3D11Device* device_ptr, ID3D11DeviceContext* device_context_ptr);
	void StartFrame();
	void RunFrame(float* tableRotation, float* tableScale, float* tableTranslation, 
		float* boxRotation, float* boxScale, float* boxTranslation,
		float &lightStrength, float &ambientStrength, float &spotAngle, 
		float *attenuationABC, float *lightProjection, float *tessFactor,
		float *emisIntens, float &emisPower);
	void ShutDownImGui();

	bool& GetToggleSpin();
	bool& GetToggleWF();
	bool& GetToggleGS();
	bool& GetToggleGaus();
	bool& GetToggleTesselate();
	bool& GetToggleEmissive();
};
#endif