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

// @brief �R���\�[����ʂɃt�H�[�}�b�g�t���������\��
// @param format �t�H�[�}�b�g(%d�Ƃ�%f�Ƃ��j
// @param �ϒ�����
// @remarks ���̊֐��̓f�o�b�O�p�ł��B�f�o�b�O���ɂ������삵�܂���B
void DebugOutputFormatString(const char* format, ...) {
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	// �E�C���h�E���j�����ꂽ��Ă΂��
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);	// OS�ɑ΂��āu�������̃A�v���͏I���v�Ɠ`����
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

	// �E�C���h�E�N���X�̐������o�^
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;	// �R�[���o�b�N�֐��̎w��
	w.lpszClassName = L"DX12Sample";			// �A�v���P�[�V������
	w.hInstance = GetModuleHandle(0);		// �n���h���̎擾

	RegisterClassEx(&w);	// �A�v���P�[�V�����N���X�i�E�C���h�E�N���X�̎w���OS�ɓ`����j

	int window_width = 800,
		window_height = 600;

	RECT wrc = { 0,0,window_width, window_height };	// �E�C���h�E�T�C�Y������

	// �֐����g���ăE�C���h�E�̃T�C�Y��␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// �E�C���h�E�I�u�W�F�N�g�𐶐�
	HWND hwnd = CreateWindow(
		w.lpszClassName,		// �N���X���w��
		L"DX12�e�X�g",			// �^�C�g���o�[�̕\��
		WS_OVERLAPPEDWINDOW,	// �^�C�g���o�[�Ƌ��E��������E�C���h�E
		CW_USEDEFAULT,			// �\��x���W��OS�ɔC����
		CW_USEDEFAULT,			// �\��y���W��OS�ɔC����
		wrc.right - wrc.left,	// �E�C���h�E�̕�
		wrc.bottom - wrc.top,	// �E�C���h�E�̍���
		nullptr,				// �e�E�C���h�E�n���h��
		nullptr,				// ���j���[�n���h��
		w.hInstance,			// �Ăяo���A�v���P�[�V�����n���h��
		nullptr);				// �ǉ��p�����[�^�[

	// �E�C���h�E�\��
	ShowWindow(hwnd, SW_SHOW);

	// �ǂꂩ������΂����̂Ńt�B�[�`���[���x���𕡐��p��
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

	// ����PC�ɐڑ����ꂽ�A�_�v�^�̗񋓗p
	std::vector<IDXGIAdapter*> adapters;
	IDXGIAdapter* tempAdapter = nullptr;

	// �S���̃A�_�v�^������
	for (int i = 0; _dxgiFactory->EnumAdapters(i, &tempAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
		adapters.push_back(tempAdapter);
	}

	for (auto w : adapters) {
		DXGI_ADAPTER_DESC adesc = {};
		w->GetDesc(&adesc);		// �A�_�v�^�[�̐����I�u�W�F�N�g�擾

		std::wstring strDesc = adesc.Description;

		// �T�������A�_�v�^�[�̖��O���m�F
		if (strDesc.find(L"NVIDIA") != std::string::npos) {
			tempAdapter = w;
			break;
		}
	}

	// Direct3D�f�o�C�X�̏�����
	D3D_FEATURE_LEVEL featurelevel;
	for (auto w : levels) {
		// �f�o�C�X�쐬
		if (D3D12CreateDevice(tempAdapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&_device)) == S_OK) {
			featurelevel = w;
			break;
		}
	}

	result = _device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));
	result = _device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));

	// �R�}���h�L���[�쐬
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};

	// �^�C���A�E�g�Ȃ�
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	// �A�_�v�^�[���P�����g��Ȃ�����0�ł����炵��
	cmdQueueDesc.NodeMask = 0;

	// �v���C�I���e�B�̐ݒ�͓��ɂȂ�
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

	// �R�}���h���X�g�ƍ��킹��
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	// �L���[����
	result = _device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));

	// �����I�u�W�F�N�g�쐬
	DXGI_SWAP_CHAIN_DESC1 swapchaindesc{};

	swapchaindesc.Width				 = window_width;
	swapchaindesc.Height			 = window_height;
	swapchaindesc.Format			 = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchaindesc.Stereo			 = false;
	swapchaindesc.SampleDesc.Count	 = 1;
	swapchaindesc.SampleDesc.Quality = 0;
	swapchaindesc.BufferUsage		 = DXGI_USAGE_BACK_BUFFER;
	swapchaindesc.BufferCount		 = 2;

	// �o�b�N�o�b�t�@�\�͐L�яk�݉\
	swapchaindesc.Scaling	 = DXGI_SCALING_STRETCH;

	// �t���b�v��͑��j��
	swapchaindesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// ���Ɏw��Ȃ�
	swapchaindesc.AlphaMode	 = DXGI_ALPHA_MODE_UNSPECIFIED;

	// �E�C���h�E<=>�t���X�N���[���̐؂�ւ��\
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

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;		// �����_�[�^�[�Q�b�g�r���[
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;						// �\����2��
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	// ���Ɏw��Ȃ�

	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	// �f�X�N���v�^�쐬
	result = _device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));



	std::vector<ID3D12Resource*> _backBuffers(swapchaindesc.BufferCount);
	for (int i = 0; i < swapchaindesc.BufferCount; i++) {
		result = _swapchain->GetBuffer(i, IID_PPV_ARGS(&_backBuffers[i]));
	}

	MSG msg = {};

	// ���C�����[�v
	while (true) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// �A�v���P�[�V�������I���Ƃ���message��WM_QUIT�ɂȂ�
		if (msg.message == WM_QUIT) {
			break;
		}
	}

	// �����N���X�͎g��Ȃ��̂ŁA�o�^��������
	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;
}