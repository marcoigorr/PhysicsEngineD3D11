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
    if (!_imgui->Initialize(hWnd, _dev, _devcon))
        return false;
    return true;
}

bool Graphics::InitD3D11(HWND hWnd)
{
    HRESULT hr;

    // Struct hold information about swap chain
    DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
    
    // Fill the swap chain description struct
    // Note: If multi-sample antialiasing is being used, all bound render targets and depth buffers must have the same sample counts and quality levels.
    scd.BufferCount = 1;                                    // one back buffer
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
    scd.OutputWindow = hWnd;                                // the window to be used
    scd.SampleDesc.Count = 1;                               // MSAA (Anti-Alias)
    scd.SampleDesc.Quality = 0;                             // The valid range is between zero and one less than the level returned by ID3D11Device::CheckMultisampleQualityLevels
    scd.Windowed = TRUE;                                    // windowed/full-screen mode
    scd.BufferDesc.Width = _wWidth;
    scd.BufferDesc.Height = _wHeight;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;     // allow full-screen switching
    scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    
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
    ID3D11Texture2D* pBackBuffer = nullptr;
    _swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    // Use the back buffer address to create the render target
    hr = _dev->CreateRenderTargetView(pBackBuffer, nullptr, &_backbuffer);
    pBackBuffer->Release();
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create render target view.");
        return false;
    }

    // Depth Stencil buffer
    D3D11_TEXTURE2D_DESC dsd;
    ZeroMemory(&dsd, sizeof(D3D11_TEXTURE2D_DESC));
    dsd.Width = _wWidth;
    dsd.Height = _wHeight;
    dsd.MipLevels = 1;
    dsd.ArraySize = 1;
    dsd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsd.SampleDesc.Count = 1;
    dsd.SampleDesc.Quality = 0;
    dsd.Usage = D3D11_USAGE_DEFAULT;
    dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    dsd.CPUAccessFlags = 0;
    dsd.MiscFlags = 0;

    // Create the texture for the depth buffer using the filled out description
    hr = _dev->CreateTexture2D(&dsd, NULL, &_depthStencilBuffer);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create depth stencil buffer.");
        return false;
    }

    // Create depth stencil state
    D3D11_DEPTH_STENCIL_DESC dsdesc;
    ZeroMemory(&dsdesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
    dsdesc.DepthEnable = true;
    dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsdesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    dsdesc.StencilEnable = true;
    dsdesc.StencilReadMask = 0xFF;
    dsdesc.StencilWriteMask = 0xFF;

    //stencil operations if pixel is front-facing
    dsdesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsdesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    dsdesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsdesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    //stencil operations if pixel is back-facing
    dsdesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsdesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    dsdesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsdesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    hr = _dev->CreateDepthStencilState(&dsdesc, &_depthStencilState);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create depth stencil state.");
        return false;
    }

    // Initailze and create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    hr = _dev->CreateDepthStencilView(_depthStencilBuffer, &depthStencilViewDesc, &_depthStencilView);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create depth stencil view.");
        return false;
    }

    // Bind the render target view and depth stencil buffer to the output render pipeline
    _devcon->OMSetRenderTargets(1, &_backbuffer, _depthStencilView);

    // Set viewport
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = _wWidth;
    viewport.Height = _wHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    _devcon->RSSetViewports(1, &viewport);

    // Create rasterizer state
    D3D11_RASTERIZER_DESC rd;
    ZeroMemory(&rd, sizeof(D3D11_RASTERIZER_DESC));
    rd.AntialiasedLineEnable = true;
    rd.MultisampleEnable = false;
    rd.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
    rd.FrontCounterClockwise = false;

    hr = _dev->CreateRasterizerState(&rd, &_rasterizerState);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create rasterizer state.");
        return false;
    }

    // Setup Blend State for transperency
    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));

    D3D11_RENDER_TARGET_BLEND_DESC rtbd;
    ZeroMemory(&rtbd, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
    rtbd.BlendEnable = true;
    rtbd.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    rtbd.BlendOp = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    blendDesc.AlphaToCoverageEnable = false;  // true if multi-sample renders
    blendDesc.RenderTarget[0] = rtbd;

    hr = _dev->CreateBlendState(&blendDesc, &_blendState);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create blend state.");
        return false;
    }

    _spriteBatch = std::make_unique<DirectX::SpriteBatch>(_devcon);
    _spriteFont = std::make_unique<DirectX::SpriteFont>(_dev, L"Data\\Fonts\\arial_14.spritefont");

    // Texture sampler
    D3D11_SAMPLER_DESC sd;
    ZeroMemory(&sd, sizeof(D3D11_SAMPLER_DESC));
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;  // x coord on texture
    sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;  // y
    sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;  // for 3d textures
    sd.MipLODBias = 0;
    sd.MaxAnisotropy = 8;
    sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
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
        {"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
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

    // Load image and create texture
    hr = D3DX11CreateShaderResourceViewFromFile(_dev, L"Data\\Textures\\circle_07.png", NULL, NULL, &_imageShaderResourceView, NULL);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create texture from file.");
        return false;
    }

    // Initialize contant buffer(s)
    hr = _cb_vs_vertexshader.Initialize(_dev, _devcon);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create constant buffer.");
        return false;
    } 
    
    hr = _cb_ps_pixelshader.Initialize(_dev, _devcon);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create constant buffer.");
        return false;
    }

    int entities = 50;

    // Create orbiting entities
    for (int i = 0; i < entities; i++)
    {
        Entity* newParticle = new Entity();
        _particles.push_back(newParticle);

        _particles[i]->Create(0.5f, 100.0f, _imageShaderResourceView, XMFLOAT3(20.0f - i * 0.45f, 20.0f + i * 0.45f, 100.0f), XMFLOAT2(0.3f, 0.0f));
        _particles[i]->Initialize(_dev, _devcon, _cb_vs_vertexshader, _cb_ps_pixelshader);
    }

    // Create gravity source
    _gravitySource.Create(1.0f, 1000.0f, _imageShaderResourceView, XMFLOAT3(0.0f, 0.0f, 100.0f), XMFLOAT2(0.0f, 0.0f));
    _gravitySource.Initialize(_dev, _devcon, _cb_vs_vertexshader, _cb_ps_pixelshader);

    _camera.SetProjectionValues(90.0f, static_cast<float>(_wWidth) / static_cast<float>(_wHeight), 0.1f, 1000.0f);

    return true;
}

void Graphics::RenderFrame(void) 
{
    // Clear the back buffer to a color
    _devcon->ClearRenderTargetView(_backbuffer, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));

    // Refresh depth stencil view
    _devcon->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    constexpr float blendFactor[] = { 0, 0, 0, 0 };
    _devcon->OMSetBlendState(_blendState, blendFactor, 0xffffffff);

    _devcon->IASetInputLayout(_pLayout);
    _devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _devcon->RSSetState(_rasterizerState);
    _devcon->OMSetDepthStencilState(_depthStencilState, 1);
    _devcon->PSSetSamplers(0, 1, &_samplerState);
    _devcon->VSSetShader(_pVS, nullptr, 0);
    _devcon->PSSetShader(_pPS, nullptr, 0);

    // Entity draw
    int nParticles = _particles.size();
    static XMFLOAT3 cameraPos;
    static XMFLOAT3 sourcePos {0.0f,0.0f,100.0f};
    
    _camera.SetPosition(cameraPos);

    _gravitySource.SetPosition(sourcePos);
    _gravitySource.Draw(_camera.GetViewMatrix() * _camera.GetProjectionMatrix());

    for (int i = 0; i < nParticles; i++)
    {
        _particles[i]->Draw(_camera.GetViewMatrix() * _camera.GetProjectionMatrix());
    }        
    
    // Text / FPS
    static int fpsCount = 0;
    static std::string fpsString = "FPS: 0";
    fpsCount += 1;
    if (_fpsTimer.GetMillisecondElapsed() > 1000.0f)
    {
        fpsString = "FPS: " + std::to_string(fpsCount);
        fpsCount = 0;
        _fpsTimer.Restart();
    }

    // Start ImGui
    _imgui->BeginRender();
    {        
        ImGui::Begin("Camera");
        {
            static float* camv[3] = { &cameraPos.x, &cameraPos.y, &cameraPos.z };
            ImGui::DragFloat3("Position (x, y, z)", *camv, 0.1f);
            if (ImGui::Button("RESET POSITION", { 110.0f,20.0f }))
            {
                cameraPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
            }
        } ImGui::End();

        ImGui::Begin("Gravity Source");
        {
            static float* srcv[3] = { &sourcePos.x, &sourcePos.y, &sourcePos.z };
            ImGui::DragFloat3("Position (x, y, z)", *srcv, 0.1f);
            if (ImGui::Button("RESET POSITION", { 110.0f,20.0f }))
            {
                sourcePos = XMFLOAT3(0.0f, 0.0f, 100.0f);
            }
        } ImGui::End();

        ImGui::Begin("Particles");
        {
            if (ImGui::Button("Delete All", { 100.0f,20.0f }) && nParticles != 0)
            {
	            _particles.clear();
            	nParticles = 0;
            }
            ImGui::Checkbox("Edit mode", &_editing);
            ImGui::Spacing();
        	for (int i = 0; i < nParticles; i++)
            {
                XMFLOAT3 particlePos = _particles[i]->GetPositionFloat3();
                std::string label = "Entity " + std::to_string(i) + " -> x: " + std::to_string(particlePos.x) + " y: " + std::to_string(particlePos.y) + " z: " + std::to_string(particlePos.z);
                ImGui::Text(label.c_str());
                ImGui::Spacing();
            }

        } ImGui::End();
    }
    _imgui->EndRender();    

    // Font Render
    _spriteBatch->Begin();
    _spriteFont->DrawString(_spriteBatch.get(), fpsString.c_str(), DirectX::XMFLOAT2(0, 0), DirectX::Colors::White, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f));
    _spriteBatch->End();

    // Switch back buffer and front buffer
    _swapchain->Present(1, 0); // 1 for VSync
}

void Graphics::CleanD3D(void)
{
    _imgui->ShutDown();

    _swapchain->SetFullscreenState(FALSE, NULL);  // switch to windowed mode

    // Close and release all existing COM objects
    if (_imageShaderResourceView) _imageShaderResourceView->Release();
    if (&_gravitySource) _gravitySource.Release();
    for (int i = 0; i < _particles.size(); i++)
    {
        if (_particles[i]) _particles[i]->Release();
    }
    if (_spriteBatch) _spriteBatch.release();
    if (_spriteFont) _spriteFont.release();
    if (_depthStencilState) _depthStencilState->Release();
    if (_depthStencilView) _depthStencilView->Release();
    if (_depthStencilBuffer) _depthStencilBuffer->Release();
    if (_blendState) _blendState->Release();
    if (_rasterizerState) _rasterizerState->Release();
    if (_pLayout) _pLayout->Release();
    if (_pVS) _pVS->Release();
    if (_pPS) _pPS->Release();
    if (_samplerState) _samplerState->Release();
    if (_cb_ps_pixelshader.GetAddressOf()) _cb_ps_pixelshader.Release();
    if (_cb_vs_vertexshader.GetAddressOf()) _cb_vs_vertexshader.Release();    
    if (_swapchain) _swapchain->Release();
    if (_backbuffer) _backbuffer->Release();
    if (_dev) _dev->Release();
    if (_devcon) _devcon->Release();
}
