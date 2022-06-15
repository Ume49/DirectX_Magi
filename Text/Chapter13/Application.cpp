#include "Application.h"
#include"PMDActor.h"
#include"Dx12Wrapper.h"
#include"PMDRenderer.h"

using namespace std;
constexpr int window_width = 1280;
constexpr int window_height = 720;

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

Application& 
Application::Instance() {
	static Application instance;
	return instance;
}

Application::Application()
{
}


Application::~Application()
{
}


bool 
Application::Initialize() {

	auto result=CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	
	_wndClass.hInstance = GetModuleHandle(nullptr);
	_wndClass.cbSize = sizeof(WNDCLASSEX);
	_wndClass.lpfnWndProc = (WNDPROC)WindowProcedure;
	_wndClass.lpszClassName = "DirectX12サンプル";
	RegisterClassEx(&_wndClass);
	
	RECT wrc = {};
	wrc.left= 0;
	wrc.top = 0;
	wrc.right = window_width;
	wrc.bottom = window_height;
	AdjustWindowRect(&wrc,WS_OVERLAPPEDWINDOW, false);


	_hwnd = CreateWindow(
		_wndClass.lpszClassName,
		"DirectX12の実験でーす",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom-wrc.top,
		nullptr,
		nullptr,
		_wndClass.hInstance,
		nullptr
	);


	if (_hwnd == 0)return false;


	_dx12.reset(new Dx12Wrapper(_hwnd));
	_pmdRenderer.reset(new PMDRenderer(_dx12));
	if (!_dx12->Init()) {
		return false;
	}
	_pmdRenderer->Init();
	_actor=make_shared< PMDActor>(_dx12, "Model/初音ミク.pmd");
	_actor->Move(-10, 0, 0);
	_actor->LoadVMDData("motion/yagokoro.vmd");
	_pmdRenderer->AddActor(_actor);


	auto ruka = make_shared<PMDActor>(_dx12, "Model/巡音ルカ.pmd");
	ruka->LoadVMDData("motion/yagokoro.vmd");
	_pmdRenderer->AddActor(ruka);

	auto haku = make_shared<PMDActor>(_dx12, "Model/弱音ハク.pmd");
	haku->Move(-5, 0, 5);
	haku->LoadVMDData("motion/yagokoro.vmd");
	_pmdRenderer->AddActor(haku);

	auto rin = make_shared<PMDActor>(_dx12, "Model/鏡音リン.pmd");
	rin->LoadVMDData("motion/yagokoro.vmd");
	rin->Move(10, 0, 10);
	_pmdRenderer->AddActor(rin);

	
	auto meiko = make_shared<PMDActor>(_dx12, "Model/咲音メイコ.pmd");
	meiko->Move(-10, 0, 10);
	meiko->LoadVMDData("motion/yagokoro.vmd");
	_pmdRenderer->AddActor(meiko);
	
	auto kaito = make_shared<PMDActor>(_dx12, "Model/カイト.pmd");
	kaito->Move(10, 0, 0);
	kaito->LoadVMDData("motion/yagokoro.vmd");
	_pmdRenderer->AddActor(kaito);
	

	_pmdRenderer->AnimationStart();
	return true;
}
///アプリケーション起動
void 
Application::Run() {
	ShowWindow(_hwnd, SW_SHOW);
	MSG msg = {};
	float fov = 3.1415926535897f / 4.0f;//π/4
	while (true) {//メインループ
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);//翻訳
			DispatchMessage(&msg);//
		}
		if (msg.message == WM_QUIT) {
			break;
		}
		BYTE keycode[256];
		GetKeyboardState(keycode);
		float x=0, y=0, z = 0;
		
		if (keycode[VK_RIGHT]&0x80) {
			x += 0.3f;
		}
		if (keycode[VK_LEFT] & 0x80) {
			x -= 0.3f;
		}
		if (keycode[VK_UP] & 0x80) {
			y += 0.3f;
		}
		if (keycode[VK_DOWN] & 0x80) {
			y -= 0.3f;
		}
		if (keycode['Z'] & 0x80) {
			fov += 0.01f;
		}
		if (keycode['X'] & 0x80) {
			fov -= 0.01f;
		}

		float px=0, py=0, pz=0;
		if (keycode['W'] & 0x80) {
			pz += 0.1f;
		}
		if (keycode['A'] & 0x80) {
			px-= 0.1f;
		}
		if (keycode['S'] & 0x80) {
			pz -= 0.1f;
		}
		if (keycode['D'] & 0x80) {
			px += 0.1f;
		}
		if (keycode['R'] & 0x80) {
			_actor->Rotate(0, 0.01f, 0);
		}
		if (keycode['T'] & 0x80) {
			_actor->Rotate(0.01f,0, 0);
		}
		if (keycode['Y'] & 0x80) {
			_actor->Rotate(0, 0, 0.01f);
		}
		_actor->Move(px, py, pz);
		
		_dx12->MoveEyePosition(x, y, z);
		_dx12->SetFov(fov);
		_dx12->PreDrawToPera1();
		_pmdRenderer->Update();
		_pmdRenderer->BeforeDraw();
		_dx12->DrawToPera1(_pmdRenderer);
		_pmdRenderer->Draw();
		_dx12->PostDrawToPera1();

		

		//_pmdRenderer->Update();
		//_pmdRenderer->BeforeDraw();
		
		_dx12->DrawHorizontalBokeh();

		_dx12->Clear();
		_dx12->Draw(_pmdRenderer);

		//_pmdRenderer->Draw();

		_dx12->Flip();
	}
}
///アプリケーション終了
void 
Application::Terminate() {
	CoUninitialize();
	UnregisterClass(_wndClass.lpszClassName, _wndClass.hInstance);
}

Size 
Application::GetWindowSize() {
	return Size(window_width,window_height);
}