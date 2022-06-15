#include<Windows.h>
#include<tchar.h>

#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include<array>
#include<vector>
#include<string>

#ifdef _DEBUG
#include<iostream>
#endif

using namespace std;

// @brief コンソール画面にフォーマット付き文字列を表示
// @param format フォーマット(%dとか%fとか）
// @param 可変長引数
// @remarks この関数はデバッグ用です。デバッグ時にしか動作しません。
void DebugOutputFormatString(const char* format, ...) {
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	// ウインドウが破棄されたら呼ばれる
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);	// OSに対して「もうこのアプリは終わる」と伝える
		return 0;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

const unsigned int window_width = 800;
const unsigned int window_height = 600;

ID3D12Device* _device = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
IDXGISwapChain4* _swapchain = nullptr;

ID3D12CommandAllocator* _cmdAllocator = nullptr;
ID3D12GraphicsCommandList* _cmdList = nullptr;
ID3D12CommandQueue* _cmdQueue = nullptr;

#ifdef _DEBUG
int main() {
#else
#include<Windows.h>
int WINAPI() WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#endif
	DebugOutputFormatString("Show window test.");
	HINSTANCE hInst = GetModuleHandle(nullptr);

	// ウインドウクラスの生成＆登録
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;	// コールバック関数の指定
	w.lpszClassName = L"DX12Sample";			// アプリケーション名
	w.hInstance = GetModuleHandle(0);		// ハンドルの取得

	RegisterClassEx(&w);	// アプリケーションクラス（ウインドウクラスの指定をOSに伝える）

	int window_width = 800,
		window_height = 600;

	RECT wrc = { 0,0,window_width, window_height };	// ウインドウサイズを決定

	// 関数を使ってウインドウのサイズを補正する
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウインドウオブジェクトを生成
	HWND hwnd = CreateWindow(
		w.lpszClassName,		// クラス名指定
		L"DX12テスト",			// タイトルバーの表示
		WS_OVERLAPPEDWINDOW,	// タイトルバーと境界線があるウインドウ
		CW_USEDEFAULT,			// 表示x座標はOSに任せる
		CW_USEDEFAULT,			// 表示y座標はOSに任せる
		wrc.right - wrc.left,	// ウインドウの幅
		wrc.bottom - wrc.top,	// ウインドウの高さ
		nullptr,				// 親ウインドウハンドル
		nullptr,				// メニューハンドル
		w.hInstance,			// 呼び出しアプリケーションハンドル
		nullptr);				// 追加パラメーター

	// ウインドウ表示
	ShowWindow(hwnd, SW_SHOW);

	// どれか一個合えばいいのでフィーチャーレベルを複数用意
	std::array<D3D_FEATURE_LEVEL, 4> levels = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory)))) {
		if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&_dxgiFactory)))) {
			return -1;
		}
	}

	HRESULT result = S_OK;

	// このPCに接続されたアダプタの列挙用
	std::vector<IDXGIAdapter*> adapters;
	IDXGIAdapter* tempAdapter = nullptr;

	// 全部のアダプタを検索
	for (int i = 0; _dxgiFactory->EnumAdapters(i, &tempAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
		adapters.push_back(tempAdapter);
	}

	for (auto w : adapters) {
		DXGI_ADAPTER_DESC adesc = {};
		w->GetDesc(&adesc);		// アダプターの説明オブジェクト取得

		std::wstring strDesc = adesc.Description;

		// 探したいアダプターの名前を確認
		if (strDesc.find(L"NVIDIA") != std::string::npos) {
			tempAdapter = w;
			break;
		}
	}

	// Direct3Dデバイスの初期化
	D3D_FEATURE_LEVEL featurelevel;
	for (auto w : levels) {
		// デバイス作成
		if (D3D12CreateDevice(tempAdapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&_device)) == S_OK) {
			featurelevel = w;
			break;
		}
	}

	result = _device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));
	result = _device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));

	// コマンドキュー作成
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};

	// タイムアウトなし
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	// アダプターを１つしか使わない時は0でいいらしい
	cmdQueueDesc.NodeMask = 0;

	// プライオリティの設定は特になし
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

	// コマンドリストと合わせる
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	// キュー生成
	result = _device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));

	// 説明オブジェクト作成
	DXGI_SWAP_CHAIN_DESC1 swapchaindesc{};

	swapchaindesc.Width				 = window_width;
	swapchaindesc.Height			 = window_height;
	swapchaindesc.Format			 = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchaindesc.Stereo			 = false;
	swapchaindesc.SampleDesc.Count	 = 1;
	swapchaindesc.SampleDesc.Quality = 0;
	swapchaindesc.BufferUsage		 = DXGI_USAGE_BACK_BUFFER;
	swapchaindesc.BufferCount		 = 2;

	// バックバッファ―は伸び縮み可能
	swapchaindesc.Scaling	 = DXGI_SCALING_STRETCH;

	// フリップ後は即破棄
	swapchaindesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// 特に指定なし
	swapchaindesc.AlphaMode	 = DXGI_ALPHA_MODE_UNSPECIFIED;

	// ウインドウ<=>フルスクリーンの切り替え可能
	swapchaindesc.Flags		 = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue,
		hwnd,
		&swapchaindesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&_swapchain
	);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;		// レンダーターゲットビュー
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;						// 表裏の2つ
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	// 特に指定なし

	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	// デスクリプタ作成
	result = _device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));



	std::vector<ID3D12Resource*> _backBuffers(swapchaindesc.BufferCount);
	for (int i = 0; i < swapchaindesc.BufferCount; i++) {
		result = _swapchain->GetBuffer(i, IID_PPV_ARGS(&_backBuffers[i]));
	}

	MSG msg = {};

	// メインループ
	while (true) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// アプリケーションが終わるときにmessageがWM_QUITになる
		if (msg.message == WM_QUIT) {
			break;
		}
	}

	// もうクラスは使わないので、登録解除する
	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;
}