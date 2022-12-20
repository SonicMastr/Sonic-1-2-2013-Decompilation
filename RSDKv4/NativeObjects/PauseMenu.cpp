#include "RetroEngine.hpp"

#if RETRO_SOFTWARE_RENDER
TextMenu pauseTextMenu;

ushort *frameBufferCopy = nullptr;

void saveFrameBuffer()
{
    if (frameBufferCopy == nullptr)
        frameBufferCopy = new ushort[GFX_LINESIZE * SCREEN_YSIZE];

    if (frameBufferCopy && Engine.frameBuffer)
        memcpy(frameBufferCopy, Engine.frameBuffer, GFX_LINESIZE * SCREEN_YSIZE * sizeof(ushort));
}

void drawFrameBuffer()
{
    if (frameBufferCopy && Engine.frameBuffer)
        memcpy(Engine.frameBuffer, frameBufferCopy, GFX_LINESIZE * SCREEN_YSIZE * sizeof(ushort));
}

void freeFrameBuffer()
{
    if (frameBufferCopy)
        delete[] frameBufferCopy;
    frameBufferCopy = nullptr;
}

void PauseMenu_Create(void *objPtr)
{
    saveFrameBuffer();

    NativeEntity_PauseMenu *pauseMenu = (NativeEntity_PauseMenu *)objPtr;
    pauseMenu->state                  = 0;
    pauseMenu->timer                  = 0;
    pauseMenu->selectedOption         = 0;
    pauseMenu->barPos                 = SCREEN_XSIZE + 64;
    pauseMenu->menu                   = &pauseTextMenu;
    MEM_ZEROP(pauseMenu->menu);

    AddTextMenuEntry(pauseMenu->menu, "RESUME");
    AddTextMenuEntry(pauseMenu->menu, "");
    AddTextMenuEntry(pauseMenu->menu, "");
    AddTextMenuEntry(pauseMenu->menu, "");
    AddTextMenuEntry(pauseMenu->menu, "RESTART");
    AddTextMenuEntry(pauseMenu->menu, "");
    AddTextMenuEntry(pauseMenu->menu, "");
    AddTextMenuEntry(pauseMenu->menu, "");
    AddTextMenuEntry(pauseMenu->menu, "EXIT");
    if (Engine.devMenu) {
        AddTextMenuEntry(pauseMenu->menu, "");
        AddTextMenuEntry(pauseMenu->menu, "");
        AddTextMenuEntry(pauseMenu->menu, "");
        AddTextMenuEntry(pauseMenu->menu, "DEV MENU");
    }
    pauseMenu->menu->alignment      = MENU_ALIGN_CENTER;
    pauseMenu->menu->selectionCount = Engine.devMenu ? 3 : 2;
    pauseMenu->menu->selection1     = 0;
    pauseMenu->menu->selection2     = 0;
    pauseMenu->lastSurfaceNo        = textMenuSurfaceNo;
    textMenuSurfaceNo               = SURFACE_COUNT - 1;

    SetPaletteEntryPacked(7, 0x08, GetPaletteEntryPacked(0, 8));
    SetPaletteEntryPacked(7, 0xFF, 0xFFFFFF);
}
void PauseMenu_Main(void *objPtr)
{
    CheckKeyDown(&inputDown);
    CheckKeyPress(&inputPress);
    NativeEntity_PauseMenu *pauseMenu = (NativeEntity_PauseMenu *)objPtr;

    int lives = GetGlobalVariableByName("player.lives");

    switch (pauseMenu->state) {
        case 0:
            // wait
            pauseMenu->barPos -= 16;
            if (pauseMenu->barPos + 64 < SCREEN_XSIZE) {
                pauseMenu->state++;
            }
            break;
        case 1:
            if (!pauseMenu->touchControls) {
                if (inputPress.up) {
                    if (pauseMenu->selectedOption - 1 < 0) {
                        if (!Engine.devMenu)
                            pauseMenu->selectedOption = 3;
                        else
                            pauseMenu->selectedOption = 4;
                    }
                    --pauseMenu->selectedOption;

                    if (pauseMenu->selectedOption == 1 && lives <= 1)
                        pauseMenu->selectedOption--;

                    PlaySfxByName("MenuMove", 0);
                }
                else if (inputPress.down) {
                    if (!Engine.devMenu)
                        pauseMenu->selectedOption = ++pauseMenu->selectedOption % 3;
                    else
                        pauseMenu->selectedOption = ++pauseMenu->selectedOption % 4;

                    if (pauseMenu->selectedOption == 1 && lives <= 1)
                        pauseMenu->selectedOption++;

                    PlaySfxByName("MenuMove", 0);
                }

                pauseMenu->menu->selection1 = pauseMenu->selectedOption * 4;

                if (inputPress.A || inputPress.start) {
                    switch (pauseMenu->selectedOption) {
                        case 0: {
                            //Engine.gameMode  = ENGINE_EXITPAUSE;
                            pauseMenu->state = 2;
                            break;
                        }
                        case 1: {
                            pauseMenu->state = 3;
                            break;
                        }
                        case 2: {
                            pauseMenu->state = 4;
                            break;
                        }
                        case 3: {
                            pauseMenu->state = 5;
                            break;
                        }
                    }
                    PlaySfxByName("MenuSelect", 0);
                }
                else if (inputPress.B) {
                    Engine.gameMode = ENGINE_EXITPAUSE;
                    PlaySfxByName("MenuBack", 0);
                    pauseMenu->state = 6;
                }

                if (CheckTouchRect(0, 0, SCREEN_XSIZE, SCREEN_YSIZE) >= 0)
                    pauseMenu->touchControls = true;
            }
            else {
                int posY = SCREEN_CENTERY - 0x30;

                int touch = CheckTouchRect(0, 0, SCREEN_XSIZE, SCREEN_YSIZE);


                if (CheckTouchRect(0, 0, pauseMenu->barPos, SCREEN_YSIZE) >= 0)
                {
                    Engine.gameMode = ENGINE_EXITPAUSE;
                    PlaySfxByName("MenuBack", 0);
                    pauseMenu->state = 6;
                }
                else {
                    if (CheckTouchRect(pauseMenu->barPos, posY - 12, SCREEN_XSIZE, posY + 12) > -1) {
                        pauseMenu->selectedOption = 0;
                    }
                    else if (pauseMenu->selectedOption == 0) {
                        if (touch < 0) {
                            PlaySfxByName("MenuSelect", 0);
                            Engine.gameMode  = ENGINE_EXITPAUSE;
                            pauseMenu->state = 2;
                        }
                        else {
                            pauseMenu->selectedOption = -1;
                        }
                    }

                    posY += 0x20;
                    if (lives > 1) {
                        if (CheckTouchRect(pauseMenu->barPos, posY - 12, SCREEN_XSIZE, posY + 12) > -1) {
                            pauseMenu->selectedOption = 1;
                        }
                        else if (pauseMenu->selectedOption == 1) {
                            if (touch < 0) {
                                PlaySfxByName("MenuSelect", 0);
                                pauseMenu->state = 3;
                            }
                            else {
                                pauseMenu->selectedOption = -1;
                            }
                        }
                    }

                    posY += 0x20;
                    if (CheckTouchRect(pauseMenu->barPos, posY - 12, SCREEN_XSIZE, posY + 12) > -1) {
                        pauseMenu->selectedOption = 2;
                    }
                    else if (pauseMenu->selectedOption == 2) {
                        if (touch < 0) {
                            PlaySfxByName("MenuSelect", 0);
                            pauseMenu->state = 4;
                        }
                        else {
                            pauseMenu->selectedOption = -1;
                        }
                    }

                    posY += 0x20;
                    if (Engine.devMenu) {
                        if (CheckTouchRect(pauseMenu->barPos, posY - 12, SCREEN_XSIZE, posY + 12) > -1) {
                            pauseMenu->selectedOption = 3;
                        }
                        else if (pauseMenu->selectedOption == 3) {
                            if (touch < 0) {
                                PlaySfxByName("MenuSelect", 0);
                                pauseMenu->state = 5;
                            }
                            else {
                                pauseMenu->selectedOption = -1;
                            }
                        }
                    }

                    pauseMenu->menu->selection1 = pauseMenu->selectedOption * 4;
                }

                if (inputPress.up || inputPress.down)
                    pauseMenu->touchControls = false;
            }
            break;
        case 2:
        case 6:
            drawFrameBuffer();
            pauseMenu->barPos += 16;
            if (pauseMenu->barPos > SCREEN_XSIZE + 64) {
                textMenuSurfaceNo = pauseMenu->lastSurfaceNo;
                RemoveNativeObject(pauseMenu);
                freeFrameBuffer();
                Engine.gameMode = ENGINE_EXITPAUSE;
                CREATE_ENTITY(RetroGameLoop);
                return;
            }
            break;
        case 3:
        case 4:
        case 5:
            // wait (again)
            pauseMenu->barPos -= 16;
            if (pauseMenu->barPos + 64 < 0) {
                textMenuSurfaceNo = pauseMenu->lastSurfaceNo;
                switch (pauseMenu->state) {
                    default: break;
                    case 3:
                        stageMode       = STAGEMODE_LOAD;
                        Engine.gameMode = ENGINE_MAINGAME;
                        if (GetGlobalVariableByName("options.gameMode") <= 1) {
                            SetGlobalVariableByName("player.lives", GetGlobalVariableByName("player.lives") - 1);
                        }
                        SetGlobalVariableByName("lampPostID", 0);
                        SetGlobalVariableByName("starPostID", 0);
                        CREATE_ENTITY(RetroGameLoop);
                        break;
                    case 4:
                        initStartMenu(0);
                        CREATE_ENTITY(RetroGameLoop);
                        break;
                    case 5:
                        CREATE_ENTITY(RetroGameLoop);
                        Engine.gameMode = ENGINE_INITDEVMENU;
                        //initDevMenu();                        
                        break;
                }
                RemoveNativeObject(pauseMenu);
                freeFrameBuffer();
                return;
            }
            break;
    }

    if (pauseMenu->menu) {
        pauseMenu->menu->selection2 = pauseMenu->menu->selection1;

        SetActivePalette(7, 0, SCREEN_YSIZE);

        DrawRectangle(pauseMenu->barPos, 0, SCREEN_XSIZE - pauseMenu->barPos, SCREEN_YSIZE, 0, 0, 0, 0xFF);
        DrawTextMenu(pauseMenu->menu, pauseMenu->barPos + 0x28, SCREEN_CENTERY - 0x30);

        SetActivePalette(0, 0, SCREEN_YSIZE);
    }
}
#else

int pauseMenuButtonCount;

void PauseMenu_Create(void *objPtr)
{
    RSDK_THIS(PauseMenu);

    // code has been here from TitleScreen_Create due to the possibility of opening the dev menu before this loads :(
#if !RETRO_USE_ORIGINAL_CODE
    int heading = -1, labelTex = -1, textTex = -1;

    if (fontList[FONT_HEADING].count <= 2) {
        if (Engine.useHighResAssets)
            heading = LoadTexture("Data/Game/Menu/Heading_EN.png", TEXFMT_RGBA4444);
        else
            heading = LoadTexture("Data/Game/Menu/Heading_EN@1x.png", TEXFMT_RGBA4444);
        LoadBitmapFont("Data/Game/Menu/Heading_EN.fnt", FONT_HEADING, heading);
    }

    if (fontList[FONT_LABEL].count <= 2) {
        if (Engine.useHighResAssets)
            labelTex = LoadTexture("Data/Game/Menu/Label_EN.png", TEXFMT_RGBA4444);
        else
            labelTex = LoadTexture("Data/Game/Menu/Label_EN@1x.png", TEXFMT_RGBA4444);
        LoadBitmapFont("Data/Game/Menu/Label_EN.fnt", FONT_LABEL, labelTex);
    }

    if (fontList[FONT_TEXT].count <= 2) {
        textTex = LoadTexture("Data/Game/Menu/Text_EN.png", TEXFMT_RGBA4444);
        LoadBitmapFont("Data/Game/Menu/Text_EN.fnt", FONT_TEXT, textTex);
    }

    switch (Engine.language) {
        case RETRO_JP:
            if (heading >= 0) {
                heading = LoadTexture("Data/Game/Menu/Heading_JA@1x.png", TEXFMT_RGBA4444);
                LoadBitmapFont("Data/Game/Menu/Heading_JA.fnt", FONT_HEADING, heading);
            }

            if (labelTex >= 0) {
                labelTex = LoadTexture("Data/Game/Menu/Label_JA@1x.png", TEXFMT_RGBA4444);
                LoadBitmapFont("Data/Game/Menu/Label_JA.fnt", FONT_LABEL, labelTex);
            }

            if (textTex >= 0) {
                textTex = LoadTexture("Data/Game/Menu/Text_JA@1x.png", TEXFMT_RGBA4444);
                LoadBitmapFont("Data/Game/Menu/Text_JA.fnt", FONT_TEXT, textTex);
            }
            break;
        case RETRO_RU:
            if (heading >= 0) {
                if (Engine.useHighResAssets)
                    heading = LoadTexture("Data/Game/Menu/Heading_RU.png", TEXFMT_RGBA4444);
                else
                    heading = LoadTexture("Data/Game/Menu/Heading_RU@1x.png", TEXFMT_RGBA4444);
                LoadBitmapFont("Data/Game/Menu/Heading_RU.fnt", FONT_HEADING, heading);
            }

            if (labelTex >= 0) {
                if (Engine.useHighResAssets)
                    labelTex = LoadTexture("Data/Game/Menu/Label_RU.png", TEXFMT_RGBA4444);
                else
                    labelTex = LoadTexture("Data/Game/Menu/Label_RU@1x.png", TEXFMT_RGBA4444);
            }
            break;
        case RETRO_KO:
            if (heading >= 0) {
                heading = LoadTexture("Data/Game/Menu/Heading_KO@1x.png", TEXFMT_RGBA4444);
                LoadBitmapFont("Data/Game/Menu/Heading_KO.fnt", FONT_HEADING, heading);
            }

            if (labelTex >= 0) {
                labelTex = LoadTexture("Data/Game/Menu/Label_KO@1x.png", TEXFMT_RGBA4444);
                LoadBitmapFont("Data/Game/Menu/Label_KO.fnt", FONT_LABEL, labelTex);
            }

            if (textTex >= 0) {
                textTex = LoadTexture("Data/Game/Menu/Text_KO.png", TEXFMT_RGBA4444);
                LoadBitmapFont("Data/Game/Menu/Text_KO.fnt", FONT_TEXT, textTex);
            }
            break;
        case RETRO_ZH:
            if (heading >= 0) {
                heading = LoadTexture("Data/Game/Menu/Heading_ZH@1x.png", TEXFMT_RGBA4444);
                LoadBitmapFont("Data/Game/Menu/Heading_ZH.fnt", FONT_HEADING, heading);
            }

            if (labelTex >= 0) {
                labelTex = LoadTexture("Data/Game/Menu/Label_ZH@1x.png", TEXFMT_RGBA4444);
                LoadBitmapFont("Data/Game/Menu/Label_ZH.fnt", FONT_LABEL, labelTex);
            }

            if (textTex >= 0) {
                textTex = LoadTexture("Data/Game/Menu/Text_ZH@1x.png", TEXFMT_RGBA4444);
                LoadBitmapFont("Data/Game/Menu/Text_ZH.fnt", FONT_TEXT, textTex);
            }
            break;
        case RETRO_ZS:
            if (heading >= 0) {
                heading = LoadTexture("Data/Game/Menu/Heading_ZHS@1x.png", TEXFMT_RGBA4444);
                LoadBitmapFont("Data/Game/Menu/Heading_ZHS.fnt", FONT_HEADING, heading);
            }

            if (labelTex >= 0) {
                labelTex = LoadTexture("Data/Game/Menu/Label_ZHS@1x.png", TEXFMT_RGBA4444);
                LoadBitmapFont("Data/Game/Menu/Label_ZHS.fnt", FONT_LABEL, labelTex);
            }

            if (textTex >= 0) {
                textTex = LoadTexture("Data/Game/Menu/Text_ZHS@1x.png", TEXFMT_RGBA4444);
                LoadBitmapFont("Data/Game/Menu/Text_ZHS.fnt", FONT_TEXT, textTex);
            }
            break;
        default: break;
    }
#endif
    pauseMenuButtonCount = PMB_COUNT;
    if (PMB_COUNT == 5 && !Engine.devMenu)
        pauseMenuButtonCount--;

    self->retroGameLoop = (NativeEntity_RetroGameLoop *)GetNativeObject(0);
    self->label         = CREATE_ENTITY(TextLabel);
    self->label->state  = TEXTLABEL_STATE_IDLE;
    self->label->z      = 0.0;
    self->label->scale  = 0.2;
    self->label->alpha  = 0;
    self->label->fontID = FONT_HEADING;
    SetStringToFont(self->label->text, strPause, FONT_HEADING);
    self->label->alignOffset = 512.0;
    self->renderRot          = DegreesToRad(22.5);
    MatrixRotateYF(&self->label->renderMatrix, DegreesToRad(22.5));
    MatrixTranslateXYZF(&self->matrix, -128.0, 80.0, 160.0);
    MatrixMultiplyF(&self->label->renderMatrix, &self->matrix);
    self->label->useRenderMatrix = true;
    self->buttonX                = ((SCREEN_CENTERX_F + -160.0) * -0.5) + -128.0;
    for (int i = 0; i < pauseMenuButtonCount; ++i) {
        NativeEntity_SubMenuButton *button = CREATE_ENTITY(SubMenuButton);
        self->buttons[i]                   = button;
        button->scale                      = 0.1;
        button->matZ                       = 0.0;
        button->matXOff                    = 512.0;
        button->textY                      = -4.0;
        self->buttonRot[i]                 = DegreesToRad(16.0);
        MatrixRotateYF(&button->matrix, DegreesToRad(16.0));
        MatrixTranslateXYZF(&self->matrix, self->buttonX, 48.0 - i * 30, 160.0);
        MatrixMultiplyF(&self->buttons[i]->matrix, &self->matrix);
        button->symbol    = 1;
        button->useMatrix = true;
    }
    if ((GetGlobalVariableByName("player.lives") <= 1 && GetGlobalVariableByName("options.gameMode") <= 1) || !activeStageList
        || GetGlobalVariableByName("options.attractMode") == 1 || GetGlobalVariableByName("options.vsMode") == 1) {
        self->buttons[PMB_RESTART]->r = 0x80;
        self->buttons[PMB_RESTART]->g = 0x80;
        self->buttons[PMB_RESTART]->b = 0x80;
    }
    SetStringToFont(self->buttons[PMB_CONTINUE]->text, strContinue, FONT_LABEL);
    SetStringToFont(self->buttons[PMB_RESTART]->text, strRestart, FONT_LABEL);
    SetStringToFont(self->buttons[PMB_SETTINGS]->text, strSettings, FONT_LABEL);
    SetStringToFont(self->buttons[PMB_EXIT]->text, strExit, FONT_LABEL);
    if (pauseMenuButtonCount == 5)
        SetStringToFont(self->buttons[PMB_DEVMENU]->text, strDevMenu, FONT_LABEL);
    self->textureCircle = LoadTexture("Data/Game/Menu/Circle.png", TEXFMT_RGBA4444);
    self->rotationY     = 0.0;
    self->rotYOff       = DegreesToRad(-16.0);
    self->matrixX       = 0.0;
    self->matrixY       = 0.0;
    self->matrixZ       = 160.0;
    self->width         = (1.75 * SCREEN_CENTERX_F) - ((SCREEN_CENTERX_F - 160) * 2);
    if (Engine.gameDeviceType == RETRO_MOBILE)
        self->textureDPad = LoadTexture("Data/Game/Menu/VirtualDPad.png", TEXFMT_RGBA8888);
    self->dpadY             = 104.0;
    self->state             = PAUSEMENU_STATE_ENTER;
    self->miniPauseDisabled = true;
    self->dpadX             = SCREEN_CENTERX_F - 76.0;
    self->dpadXSpecial      = SCREEN_CENTERX_F - 52.0;
}
void PauseMenu_Main(void *objPtr)
{
    RSDK_THIS(PauseMenu);
    switch (self->state) {
        case PAUSEMENU_STATE_SETUP: {
            self->timer += Engine.deltaTime;
            if (self->timer > 1.0) {
                self->timer = 0.0;
                self->state = PAUSEMENU_STATE_ENTER;
            }
            break;
        }
        case PAUSEMENU_STATE_ENTER: {
            if (self->unusedAlpha < 0x100)
                self->unusedAlpha += 8;
            self->timer += Engine.deltaTime * 2;
            self->label->alignOffset = self->label->alignOffset / (1.125 * (60.0 * Engine.deltaTime));
            self->label->alpha       = (256.0 * self->timer);
            for (int i = 0; i < pauseMenuButtonCount; ++i)
                self->buttons[i]->matXOff += ((-176.0 - self->buttons[i]->matXOff) / (16.0 * (60.0 * Engine.deltaTime)));
            self->matrixX += ((self->width - self->matrixX) / ((60.0 * Engine.deltaTime) * 12.0));
            self->matrixZ += ((512.0 - self->matrixZ) / ((60.0 * Engine.deltaTime) * 12.0));
            self->rotationY += ((self->rotYOff - self->rotationY) / (16.0 * (60.0 * Engine.deltaTime)));
            if (self->timer > 1.0) {
                self->timer = 0.0;
                self->state = PAUSEMENU_STATE_MAIN;
            }
            break;
        }
        case PAUSEMENU_STATE_MAIN: {
            CheckKeyDown(&inputDown);
            CheckKeyPress(&inputPress);
            if (usePhysicalControls) {
                if (touches > 0) {
                    usePhysicalControls = false;
                }
                else {
                    if (inputPress.up) {
                        PlaySfxByName("Menu Move", false);
                        self->buttonSelected--;
                        if (self->buttonSelected < PMB_CONTINUE)
                            self->buttonSelected = pauseMenuButtonCount - 1;
                    }
                    else if (inputPress.down) {
                        PlaySfxByName("Menu Move", false);
                        self->buttonSelected++;
                        if (self->buttonSelected >= pauseMenuButtonCount)
                            self->buttonSelected = PMB_CONTINUE;
                    }
                    for (int i = 0; i < pauseMenuButtonCount; ++i) self->buttons[i]->b = self->buttons[i]->r;
                    self->buttons[self->buttonSelected]->b = 0;
                    if (self->buttons[self->buttonSelected]->g > 0x80 && (inputPress.start || inputPress.A)) {
                        PlaySfxByName("Menu Select", false);
                        self->buttons[self->buttonSelected]->state = SUBMENUBUTTON_STATE_FLASHING2;
                        self->buttons[self->buttonSelected]->b     = 0xFF;
                        self->state                                = PAUSEMENU_STATE_ACTION;
                    }
                }
            }
            else {
                for (int i = 0; i < pauseMenuButtonCount; ++i) {
                    if (touches > 0) {
                        if (self->buttons[i]->g > 0x80)
                            self->buttons[i]->b = (CheckTouchRect(-80.0, 48.0 - i * 30, 112.0, 12.0) < 0) * 0xFF;
                        else
                            self->buttons[i]->b = 0x80;
                    }
                    else if (!self->buttons[i]->b) {
                        self->buttonSelected = i;
                        PlaySfxByName("Menu Select", false);
                        self->buttons[i]->state = SUBMENUBUTTON_STATE_FLASHING2;
                        self->buttons[i]->b     = 0xFF;
                        self->state             = PAUSEMENU_STATE_ACTION;
                        break;
                    }
                }

                if (self->state == PAUSEMENU_STATE_MAIN && (inputDown.up || inputDown.down)) {
                    self->buttonSelected = PMB_CONTINUE;
                    usePhysicalControls  = true;
                }
            }
            if (touches > 0) {
                if (!self->miniPauseDisabled && CheckTouchRect(SCREEN_CENTERX_F, SCREEN_CENTERY_F, 112.0, 24.0) >= 0) {
                    self->buttonSelected = PMB_CONTINUE;
                    PlaySfxByName("Resume", false);
                    self->state = PAUSEMENU_STATE_ACTION;
                }
            }
            else {
                self->miniPauseDisabled = false;
                if (self->makeSound) {
                    PlaySfxByName("Menu Select", false);
                    self->makeSound = false;
                }
            }
            break;
        }
        case PAUSEMENU_STATE_CONTINUE: {
            self->label->alignOffset += 10.0 * (60.0 * Engine.deltaTime);
            self->timer += Engine.deltaTime * 2;
            for (int i = 0; i < pauseMenuButtonCount; ++i) self->buttons[i]->matXOff += ((12.0 + i) * (60.0 * Engine.deltaTime));
            self->matrixX += ((-self->matrixX) / (5.0 * (60.0 * Engine.deltaTime)));
            self->matrixZ += ((160.0 - self->matrixZ) / (5.0 * (60.0 * Engine.deltaTime)));
            self->rotationY += ((self->rotYOff - self->rotationY) / ((60.0 * Engine.deltaTime) * 6.0));

            if (self->timer > 0.9) {
                mixFiltersOnJekyll = true;
                RenderRetroBuffer(64, 160.0);
                if (Engine.gameDeviceType == RETRO_MOBILE) {
                    if (activeStageList == STAGELIST_SPECIAL)
                        RenderImage(self->dpadXSpecial, self->dpadY, 160.0, 0.25, 0.25, 32.0, 32.0, 64.0, 64.0, 160.0, 258.0, 255, self->textureDPad);
                    else
                        RenderImage(self->dpadX, self->dpadY, 160.0, 0.25, 0.25, 32.0, 32.0, 64.0, 64.0, 160.0, 258.0, 255, self->textureDPad);
                }
                self->timer = 0.0;
                ClearNativeObjects();
                CREATE_ENTITY(RetroGameLoop);
                if (Engine.gameDeviceType == RETRO_MOBILE)
                    CREATE_ENTITY(VirtualDPad)->pauseAlpha = 255;
                ResumeSound();
                Engine.gameMode = ENGINE_MAINGAME;
                return;
            }
            break;
        }
        case PAUSEMENU_STATE_ACTION: {
            if (!self->buttons[self->buttonSelected]->state) {
                switch (self->buttonSelected) {
                    case PMB_CONTINUE:
                        self->state   = PAUSEMENU_STATE_CONTINUE;
                        self->rotYOff = 0.0;
                        break;
                    case PMB_RESTART:
                        self->dialog = CREATE_ENTITY(DialogPanel);
                        SetStringToFont(self->dialog->text, GetGlobalVariableByName("options.gameMode") ? strRestartMessage : strNSRestartMessage,
                                        FONT_TEXT);
                        self->state = PAUSEMENU_STATE_RESTART;
                        break;
                    case PMB_SETTINGS:
                        self->state        = PAUSEMENU_STATE_ENTERSUBMENU;
                        self->rotInc       = 0.0;
                        self->renderRotMax = DegreesToRad(-90.0);
                        for (int i = 0; i < pauseMenuButtonCount; ++i) {
                            self->rotMax[i]     = DegreesToRad(-90.0);
                            self->buttonRotY[i] = 0.02 * (i + 1);
                        }

                        break;
                    case PMB_EXIT:
                        self->dialog = CREATE_ENTITY(DialogPanel);
                        SetStringToFont(self->dialog->text, GetGlobalVariableByName("options.gameMode") ? strExitMessage : strNSExitMessage,
                                        FONT_TEXT);
                        self->state = PAUSEMENU_STATE_EXIT;
                        if (Engine.gameType == GAME_SONIC1)
                            SetGlobalVariableByName("timeAttack.result", 1000000);
                        break;
#if !RETRO_USE_ORIGINAL_CODE
                    case PMB_DEVMENU:
                        self->state = PAUSEMENU_STATE_DEVMENU;
                        self->timer = 0.0;
                        break;
#endif
                    default: break;
                }
            }
            break;
        }
        case PAUSEMENU_STATE_ENTERSUBMENU: {
            if (self->renderRot > self->renderRotMax) {
                self->rotInc -= 0.0025 * (Engine.deltaTime * 60.0);
                self->renderRot += (Engine.deltaTime * 60.0) * self->rotInc;
                self->rotInc -= 0.0025 * (Engine.deltaTime * 60.0);
                MatrixRotateYF(&self->label->renderMatrix, self->renderRot);
                MatrixTranslateXYZF(&self->matrix, self->buttonX, 80.0, 160.0);
                MatrixMultiplyF(&self->label->renderMatrix, &self->matrix);
            }
            for (int i = 0; i < pauseMenuButtonCount; ++i) {
                if (self->buttonRot[i] > self->rotMax[i]) {
                    self->buttonRotY[i] -= 0.0025 * (60.0 * Engine.deltaTime);
                    if (self->buttonRotY[i] < 0.0) {
                        self->buttonRot[i] += ((60.0 * Engine.deltaTime) * self->buttonRotY[i]);
                    }
                    self->buttonRotY[i] -= 0.0025 * (60.0 * Engine.deltaTime); // do it again ????
                    MatrixRotateYF(&self->buttons[i]->matrix, self->buttonRot[i]);
                    MatrixTranslateXYZF(&self->matrix, self->buttonX, 48.0 - i * 30, 160.0);
                    MatrixMultiplyF(&self->buttons[i]->matrix, &self->matrix);
                }
            }
            if (self->rotMax[pauseMenuButtonCount - 1] >= self->buttonRot[pauseMenuButtonCount - 1]) {
                self->state = PAUSEMENU_STATE_SUBMENU;

                self->rotInc       = 0.0;
                self->renderRotMax = DegreesToRad(22.5);
                for (int i = 0; i < pauseMenuButtonCount; ++i) {
                    self->rotMax[i]     = DegreesToRad(16.0);
                    self->buttonRotY[i] = -0.02 * (i + 1);
                }
                if (self->buttonSelected == 2) {
                    self->settingsScreen              = CREATE_ENTITY(SettingsScreen);
                    self->settingsScreen->optionsMenu = (NativeEntity_OptionsMenu *)self;
                    self->settingsScreen->isPauseMenu = 1;
                }
            }
            self->matrixX += ((1024.0 - self->matrixX) / ((60.0 * Engine.deltaTime) * 16.0));
            break;
        }
        case PAUSEMENU_STATE_SUBMENU: break;
        case PAUSEMENU_STATE_EXITSUBMENU: {
            if (self->renderRotMax > self->renderRot) {
                self->rotInc += 0.0025 * (Engine.deltaTime * 60.0);
                self->renderRot += (Engine.deltaTime * 60.0) * self->rotInc;
                self->rotInc += 0.0025 * (Engine.deltaTime * 60.0);
                MatrixRotateYF(&self->label->renderMatrix, self->renderRot);
                MatrixTranslateXYZF(&self->matrix, self->buttonX, 80.0, 160.0);
                MatrixMultiplyF(&self->label->renderMatrix, &self->matrix);
            }

            for (int i = 0; i < pauseMenuButtonCount; ++i) {
                if (self->rotMax[i] > self->buttonRot[i]) {
                    self->buttonRotY[i] += 0.0025 * (60.0 * Engine.deltaTime);
                    if (self->buttonRotY[i] > 0.0)
                        self->buttonRot[i] += ((60.0 * Engine.deltaTime) * self->buttonRotY[i]);
                    self->buttonRotY[i] += 0.0025 * (60.0 * Engine.deltaTime);
                    if (self->buttonRot[i] > self->rotMax[i])
                        self->buttonRot[i] = self->rotMax[i];
                    MatrixRotateYF(&self->buttons[i]->matrix, self->buttonRot[i]);
                    MatrixTranslateXYZF(&self->matrix, self->buttonX, 48.0 - i * 30, 160.0);
                    MatrixMultiplyF(&self->buttons[i]->matrix, &self->matrix);
                }
            }

            float div = (60.0 * Engine.deltaTime) * 16.0;
            self->matrixX += (((self->width - 32.0) - self->matrixX) / div);
            if (self->width - 16.0 > self->matrixX) {
                self->matrixX = self->width - 16.0;
                self->state   = PAUSEMENU_STATE_MAIN;
            }
            break;
        }
        case PAUSEMENU_STATE_RESTART: {
            if (self->dialog->selection == DLG_YES) {
                self->state     = PAUSEMENU_STATE_SUBMENU;
                Engine.gameMode = ENGINE_EXITPAUSE;
                stageMode       = STAGEMODE_LOAD;
                if (GetGlobalVariableByName("options.gameMode") <= 1) {
                    SetGlobalVariableByName("player.lives", GetGlobalVariableByName("player.lives") - 1);
                }
                if (activeStageList != STAGELIST_SPECIAL) {
                    SetGlobalVariableByName("lampPostID", 0);
                    SetGlobalVariableByName("starPostID", 0);
                }
                self->dialog->state = DIALOGPANEL_STATE_IDLE;
                StopMusic(true);
                CREATE_ENTITY(FadeScreen);
                break;
            }
            if (self->dialog->selection == DLG_NO)
                self->state = PAUSEMENU_STATE_MAIN;
            break;
        }
        case PAUSEMENU_STATE_EXIT: {
            if (self->dialog->selection == DLG_YES) {
                self->state = PAUSEMENU_STATE_SUBMENU;
#if !RETRO_USE_ORIGINAL_CODE
                if (skipStartMenu) {
                    Engine.gameMode                  = ENGINE_MAINGAME;
                    self->dialog->state              = DIALOGPANEL_STATE_IDLE;
                    NativeEntity_FadeScreen *fadeout = CREATE_ENTITY(FadeScreen);
                    fadeout->state                   = FADESCREEN_STATE_GAMEFADEOUT;
                    activeStageList                  = STAGELIST_PRESENTATION;
                    stageListPosition                = 0;
                    stageMode                        = STAGEMODE_LOAD;
                }
                else {
#endif
                    Engine.gameMode     = (GetGlobalVariableByName("options.gameMode") > 1) + ENGINE_ENDGAME;
                    self->dialog->state = DIALOGPANEL_STATE_IDLE;
                    CREATE_ENTITY(FadeScreen);
#if !RETRO_USE_ORIGINAL_CODE
                }
#endif
            }
            else {
                if (self->dialog->selection == DLG_YES || self->dialog->selection == DLG_NO || self->dialog->selection == DLG_OK) {
                    self->state   = PAUSEMENU_STATE_MAIN;
                    self->unused2 = 50;
                }
            }
            break;
        }
#if !RETRO_USE_ORIGINAL_CODE
        case PAUSEMENU_STATE_DEVMENU:
            self->timer += Engine.deltaTime;
            if (self->timer > 0.5) {
                if (!self->devMenuFade) {
                    self->devMenuFade        = CREATE_ENTITY(FadeScreen);
                    self->devMenuFade->state = FADESCREEN_STATE_FADEOUT;
                }
                if (!self->devMenuFade->delay || self->devMenuFade->timer >= self->devMenuFade->delay) {
                    ClearNativeObjects();
                    RenderRect(-SCREEN_CENTERX_F, SCREEN_CENTERY_F, 160.0, SCREEN_XSIZE_F, SCREEN_YSIZE_F, 0, 0, 0, 255);
                    CREATE_ENTITY(RetroGameLoop);
                    if (Engine.gameDeviceType == RETRO_MOBILE)
                        CREATE_ENTITY(VirtualDPad);
                    Engine.gameMode = ENGINE_INITDEVMENU;
                    return;
                }
            }
            break;
#endif
        default: break;
    }

    SetRenderBlendMode(RENDER_BLEND_NONE);
    NewRenderState();
    MatrixRotateYF(&self->matrixTemp, self->rotationY);
    MatrixTranslateXYZF(&self->matrix, self->matrixX, self->matrixY, self->matrixZ);
    MatrixMultiplyF(&self->matrixTemp, &self->matrix);
    SetRenderMatrix(&self->matrixTemp);
    RenderRect(-SCREEN_CENTERX_F - 4.0, SCREEN_CENTERY_F + 4.0, 0.0, SCREEN_XSIZE_F + 8.0, SCREEN_YSIZE_F + 8.0, 0, 0, 0, 255);
    RenderRetroBuffer(64, 0.0);
    NewRenderState();
    SetRenderMatrix(NULL);
    if (Engine.gameDeviceType == RETRO_MOBILE && self->state != PAUSEMENU_STATE_SUBMENU) {
        if (activeStageList == STAGELIST_SPECIAL)
            RenderImage(self->dpadXSpecial, self->dpadY, 160.0, 0.25, 0.25, 32.0, 32.0, 64.0, 64.0, 160.0, 258.0, 255, self->textureDPad);
        else
            RenderImage(self->dpadX, self->dpadY, 160.0, 0.25, 0.25, 32.0, 32.0, 64.0, 64.0, 160.0, 258.0, 255, self->textureDPad);
    }
    SetRenderBlendMode(RENDER_BLEND_ALPHA);
    NewRenderState();
}
#endif //RETRO_SOFTWARE_RENDER
