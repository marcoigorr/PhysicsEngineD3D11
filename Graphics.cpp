#include "Graphics.h"

bool Graphics::Initialize(HWND hWnd, int width, int height)
{
    _wWidth = width;
    _wHeight = height;

    _fpsTimer.Start();
    
    if (!this->InitD3D11(hWnd))
        return false;

    if (!this->InitPipeline())
        return false;

    if (!this->InitGraphicsD3D11())
        return false;

    if (!this->InitImGui(hWnd))
        return false;

    return true;
}

bool Graphics::InitImGui(HWND hWnd)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.WantSaveIniSettings = false;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowMinSize = ImVec2(550, 200);
    style.WindowTitleAlign = ImVec2(0.50f, 0.50f); // Title
    // style.WindowPadding = ImVec2(20.0f, 20.0f);
    // style.WindowRounding = 9.0f;
    // style.FrameRounding = 12.0f;
    // style.FramePadding = ImVec2(17.0f, 4.0f);
    // style.TabRounding = 9.0f;
    // style.GrabRounding = 10.0f;
    // style.GrabMinSize = 15.0f;
    // style.ScrollbarSize = 17.0f;
    // style.ItemSpacing = ImVec2(13.0f, 4.0f);
    // style.ItemInnerSpacing = ImVec2(10.0f, 8.0f);

    if (!ImGui_ImplWin32_Init(hWnd))
    {
        ErrorLogger::Log("ImGui_ImplWin32_Init, failed to initialize ImGui.");
        return false;
    }

    if (!ImGui_ImplDX11_Init(_dev, _devcon))
    {
        ErrorLogger::Log("ImGui_ImplDX11_Init, failed to initialize ImGui.");
        return false;
    }

    return true;
}

bool Graphics::InitD3D11(HWND hWnd)
{
    // Struct hold information about swap chain
    DXGI_SWAP_CHAIN_DESC scd;

    // Set scd struct to NULL
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
    
    // Fill the swap chain description struct
    scd.BufferCount = 1;                                    // one back buffer
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
    scd.OutputWindow = hWnd;                                // the window to be used
    scd.SampleDesc.Count = 8;                               // MSAA (Anti-Alias)
    scd.Windowed = TRUE;                                    // windowed/full-screen mode
    scd.BufferDesc.Width = _wWidth;
    scd.BufferDesc.Height = _wHeight;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;     // allow full-screen switching

    HRESULT hr;   
    
    // Create device, device context and swap chain using the information in the scd struct
    hr = D3D11CreateDeviceAndSwapChain( NULL,
                                        D3D_DRIVER_TYPE_HARDWARE,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL,
                                        D3D11_SDK_VERSION,
                                        &scd,
                                        &_swapchain,
                                        &_dev,
                                        NULL,
                                        &_devcon);

    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create device and swapchain.");
        return false;
    }

    // Get address of back buffer
    ID3D11Texture2D* pBackBuffer;
    _swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    // Use the back buffer address to create the render target
    hr = _dev->CreateRenderTargetView(pBackBuffer, NULL, &_backbuffer);
    pBackBuffer->Release();
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create render target view.");
        return false;
    }

    // Set render target as the back buffer
    _devcon->OMSetRenderTargets(1, &_backbuffer, NULL);

    // Set viewport
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = _wWidth;
    viewport.Height = _wHeight;

    _devcon->RSSetViewports(1, &viewport);

    // Create rasterizer state
    D3D11_RASTERIZER_DESC rd;
    ZeroMemory(&rd, sizeof(D3D11_RASTERIZER_DESC));
    rd.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
    hr = _dev->CreateRasterizerState(&rd, &_rasterizerState);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create rasterizer state.");
        return false;
    }

    _spriteBatch = std::make_unique<DirectX::SpriteBatch>(_devcon);
    _spriteFont = std::make_unique<DirectX::SpriteFont>(_dev, L"Data\\Fonts\\arial_14.spritefont");

    // Texture
    D3D11_SAMPLER_DESC sd;
    ZeroMemory(&sd, sizeof(D3D11_SAMPLER_DESC));
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;  // x coord on texture
    sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;  // y
    sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;  // for 3d textures
    sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sd.MinLOD = 0;  // Level of detail
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    
    hr = _dev->CreateSamplerState(&sd, &_samplerState);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create sampler state.");
        return false; 
    }

    return true;
}

bool Graphics::InitPipeline(void)
{
    // Load and compile the two shaders
    ID3D10Blob* VS, * PS;  // buffer with compiled code of the shader (COM obj)
    
    HRESULT hr;
    hr = D3DX11CompileFromFile(L"vertexshader.hlsl", 0, 0, "main", "vs_5_0", 0, 0, 0, &VS, 0, 0);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to compile vertex shader.");
        return false;
    }

    hr = D3DX11CompileFromFile(L"pixelshader.hlsl", 0, 0, "main", "ps_5_0", 0, 0, 0, &PS, 0, 0);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to compile pixel shader.");
        return false;
    }

    // Encapsulate shaders into shader objects
    hr = _dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &_pVS);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create vertex shader.");
        return false;
    }
    
    hr = _dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &_pPS);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create pixel shader.");
        return false;
    }

    // Creating input layout to let gpu organize data properly
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
    };

    hr = _dev->CreateInputLayout(ied, ARRAYSIZE(ied), VS->GetBufferPointer(), VS->GetBufferSize(), &_pLayout);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create Input layout.");
        return false;
    }

    return true;
}

bool Graphics::InitGraphicsD3D11(void)
{
    HRESULT hr;     

    hr = _cb_vs_vertexshader.Initialize(_dev, _devcon);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create constant buffer.");
        return false;
    } 

    hr = DirectX::CreateWICTextureFromFile(_dev, _devcon, L"Data\\Textures\\particle.png", nullptr, &_particleTexture);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create wic texture from file.");
        return false;
    }

    // Initialize Entity
    if (!_entity.Initialize(_dev, _devcon, _particleTexture, _cb_vs_vertexshader))
        return false;

    _camera.SetProjectionValues(90.0f, static_cast<float>(_wWidth) / static_cast<float>(_wHeight), 0.1f, 1000.0f);

    return true;
}

void Graphics::RenderFrame(void)
{
    // Clear the back buffer to a color
    _devcon->ClearRenderTargetView(_backbuffer, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));

    _devcon->IASetInputLayout(_pLayout);
    _devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _devcon->RSSetState(_rasterizerState);
    _devcon->PSSetSamplers(0, 1, &_samplerState);
    _devcon->VSSetShader(_pVS, 0, 0);
    _devcon->PSSetShader(_pPS, 0, 0);

    static XMFLOAT3 cameraPos = XMFLOAT3(0.0f, 0.0f, -20.0f);
    static XMFLOAT3 entityPos;
    static bool isEditing = false;
    {
        _camera.SetPosition(cameraPos);
        if (isEditing)
            _entity.SetPosition(entityPos);
        _entity.Draw(_camera.GetViewMatrix() * _camera.GetProjectionMatrix());
    }

    // Text / fps
    static int fpsCount = 0;
    static std::string fpsString = "FPS: 0";
    fpsCount += 1;
    if (_fpsTimer.GetMillisecondElapsed() > 1000.0f)
    {
        fpsString = "FPS: " + std::to_string(fpsCount);
        fpsCount = 0;
        _fpsTimer.Restart();
    }

    // Start ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    {
        // ImGui::SetNextWindowSize(ImVec2(500, 200));
        ImGui::Begin("Window");
        {

            ImGui::Text(fpsString.c_str());
            static float* cam[3] = { &cameraPos.x, &cameraPos.y, &cameraPos.z };
            ImGui::SliderFloat3("Camera Position (x, y, z)", *cam, -30.0f, 30.0f, "%0.1f");
            if (ImGui::Button("RESET", { 50.0f,20.0f }))
            {
                cameraPos.x = 0.0f;
                cameraPos.y = 0.0f;
                cameraPos.z = -20.0f;
            }

            ImGui::Spacing();
            
            ImGui::Checkbox("Enable edit", &isEditing);
            if (isEditing)
            {
                static float* ent[3] = { &entityPos.x, &entityPos.y, &entityPos.z };
                ImGui::SliderFloat3("Entity Position (x, y, z)", *ent, -10.0f, 10.0f, "%0.1f", 0);
            }            

        } ImGui::End();           
    }

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Font Render
    _spriteBatch->Begin();
    _spriteFont->DrawString(_spriteBatch.get(), fpsString.c_str(), DirectX::XMFLOAT2(0, 0), DirectX::Colors::White, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f));
    _spriteBatch->End();

    // Switch back buffer and front buffer
    _swapchain->Present(0, 0); // 1 for VSync
}

void Graphics::CleanD3D(void)
{
    _swapchain->SetFullscreenState(FALSE, NULL);    // switch to windowed mode

    // close and release all existing COM objects
    if (_particleTexture) _particleTexture->Release();
    if (&_entity) _entity.Release();
    if (_spriteBatch) _spriteBatch.release();
    if (_spriteFont) _spriteFont.release();
    if (_rasterizerState) _rasterizerState->Release();
    if (_pLayout) _pLayout->Release();
    if (_pVS) _pVS->Release();
    if (_pPS) _pPS->Release();
    if (_particleTexture) _particleTexture->Release();
    if (_samplerState) _samplerState->Release();
    if (_cb_vs_vertexshader.GetAddressOf()) _cb_vs_vertexshader.Release();    
    if (_swapchain) _swapchain->Release();
    if (_backbuffer) _backbuffer->Release();
    if (_dev) _dev->Release();
    if (_devcon) _devcon->Release();
}
