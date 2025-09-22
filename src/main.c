//------------------------------------------------------------------------------
// Grupo:
//
// Gabriel Shihao Chen Yin - 10408981 

// Enrico Najjar Galdeano - 10402924 

// Samuel Lopes Pereira - 10403767 

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>


//------------------------------------------------------------------------------
// Custom types, structs, constants, etc.
//------------------------------------------------------------------------------
static const char *WINDOW_TITLE = "Imagem";
static const char *CHILD_WINDOW_TITLE = "Histograma";

enum constants
{
  DEFAULT_WINDOW_WIDTH = 427,
  DEFAULT_WINDOW_HEIGHT = 320,
  INFO_HEIGHT = DEFAULT_WINDOW_HEIGHT/3
  
};

typedef struct MyWindow MyWindow;
struct MyWindow
{
  SDL_Window *window;
  SDL_Renderer *renderer;
};

typedef struct MyImage MyImage;
struct MyImage
{
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_FRect rect;
};


/* 
  Criadas para o projeto 
*/
//Strutura para o botao
typedef struct Button Button;
struct Button {
    SDL_FRect rect;
    char* text;
} ;

const SDL_Color NEUTRAL_COLOR = { 35, 86, 252, 255 };  
const SDL_Color HOVER_COLOR   = { 75, 116, 250, 255 };  
const SDL_Color CLICKED_COLOR = {35, 70, 184, 255 };   

//------------------------------------------------------------------------------
// Globals 
//------------------------------------------------------------------------------
static MyWindow g_window = { .window = NULL, .renderer = NULL };
static MyImage g_image = {
  .surface = NULL,
  .texture = NULL,
  .rect = { .x = 0.0f, .y = 0.0f, .w = 0.0f, .h = 0.0f }
};


static SDL_Cursor *defaultMouseCursor = NULL;
static SDL_Cursor *hourglassMouseCursor = NULL;

/* 
  Globais criadas para o projeto 
*/
//Botao
static Button button = {
  .text = "Original",
  .rect = { .x = 0.0f, .y = 0.0f, .w = 0.0f, .h = 0.0f }
};



// Janela filha
static MyWindow child_window = { .window = NULL, .renderer = NULL };

//Fonte do texto
TTF_Font* fonte;

//------------------------------------------------------------------------------
// Function declaration
//------------------------------------------------------------------------------
static bool MyWindow_initialize(MyWindow *window, const char *title, int width, int height, SDL_WindowFlags window_flags);
static void MyWindow_destroy(MyWindow *window);
static void MyImage_destroy(MyImage *image);
static bool MyImage_update_texture_with_surface(MyImage* image, SDL_Renderer *renderer, SDL_Surface *surface);
static bool MyImage_restore_texture(MyImage* image, SDL_Renderer *renderer);

/**
 * Carrega a imagem indicada no parâmetro `filename` e a converte para o formato
 * RGBA32, eliminando dependência do formato original da imagem. A imagem
 * carregada é armazenada em output_image.
 * Caso ocorra algum erro no processo, a função retorna false.
 */
static bool load_rgba32(const char *filename, SDL_Renderer *renderer, MyImage *output_image);


static SDL_AppResult initialize(void);
static void shutdown(void);
static void render(void);
static void loop(void);

/* 
  Funcoes criadas para o projeto 
*/



//Transforma a imagem em escala de cinza
static void MyImage_GrayScale(SDL_Renderer *renderer, MyImage *image);

//Verifica se uma imagem esta em escala de cinza
static bool MyImage_IsGrayScale(MyImage *image);

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

bool load_rgba32(const char *caminho, SDL_Renderer *renderer, MyImage *output_image)
{
  SDL_Log(">>> load_rgba32(\"%s\")", caminho);

  //verifica se o caminho  é valido
  if (!caminho)
  {
    SDL_Log("\t*** Erro: Nome do arquivo inválido (caminho == NULL).");
    SDL_Log("<<< load_rgba32(\"%s\")", caminho);
    return false;
  }

  //Verifica se extensao é valida
  const char *extensao = strrchr(caminho, '.');
  if (!extensao || (strcmp(extensao, ".png") != 0 &&
    strcmp(extensao, ".jpg") != 0 &&
    strcmp(extensao, ".bmp") != 0))
  {
    SDL_Log("\t*** Erro: Extensão de arquivo inválida. Apenas .png, .jpg e .bmp são permitidos.");
    SDL_Log("<<< load_rgba32(\"%s\")", caminho);
    return false;
  }

  if (!renderer)
  {
    SDL_Log("\t*** Erro: Renderer inválido (renderer == NULL).");
    SDL_Log("<<< load_rgba32(\"%s\")", caminho);
    return false;
  }

  if (!output_image)
  {
    SDL_Log("\t*** Erro: Imagem de saída inválida (output_image == NULL).");
    SDL_Log("<<< load_rgba32(\"%s\")", caminho);
    return false;
  }

  MyImage_destroy(output_image);

  SDL_Log("\tCarregando imagem \"%s\" em uma superfície...", caminho);
  SDL_Surface *surface = IMG_Load(caminho);
  if (!surface)
  {
    SDL_Log("\t*** Erro ao carregar a imagem: %s", SDL_GetError());
    SDL_Log("<<< load_rgba32(\"%s\")", caminho);
    return false;
  }

  SDL_Log("\tConvertendo superfície para formato RGBA32...");
  output_image->surface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
  SDL_DestroySurface(surface);
  if (!output_image->surface)
  {
    SDL_Log("\t*** Erro ao converter superfície para formato RGBA32: %s", SDL_GetError());
    SDL_Log("<<< load_rgba32(\"%s\")", caminho);
    return false;
  }

  SDL_Log("\tCriando textura a partir da superfície...");
  if (!MyImage_update_texture_with_surface(output_image, renderer, output_image->surface))
  {
    SDL_Log("\t*** Erro ao criar textura.");
    SDL_Log("<<< load_rgba32(\"%s\")", caminho);
    return false;
  }

  SDL_Log("<<< load_rgba32(\"%s\")", caminho);
  return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SDL_AppResult initialize(void)
{
  SDL_Log(">>> initialize()");

  SDL_Log("\tIniciando SDL...");
  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    SDL_Log("\t*** Erro ao iniciar a SDL: %s", SDL_GetError());
    SDL_Log("<<< initialize()");
    return SDL_APP_FAILURE;
  }

  if (!TTF_Init())
  {
    SDL_Log("\t*** Erro ao iniciar a TTF");
    SDL_Log("<<< initialize()");
    return SDL_APP_FAILURE;
  }

  //Definir fonte para texto
  fonte = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 14);
  if (!fonte) {
      SDL_Log("TTF_OpenFont Error\n");
      SDL_Log("<<< initialize()");
  }

  SDL_Log("<<< initialize()");
  return SDL_APP_CONTINUE;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void shutdown(void)
{
  SDL_Log(">>> shutdown()");

  SDL_Log("Destruindo cursores do mouse...");
  SDL_DestroyCursor(hourglassMouseCursor);
  SDL_DestroyCursor(defaultMouseCursor);
  defaultMouseCursor = NULL;
  hourglassMouseCursor = NULL;

  MyImage_destroy(&g_image);
  MyWindow_destroy(&g_window);
  MyWindow_destroy(&child_window);

  SDL_Log("\tEncerrando SDL e TTF...");
  TTF_Quit();
  SDL_Quit();

  SDL_Log("<<< shutdown()");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------


//******************************************************************************
// **IMPLEMENTACOES DO PROJETO 1
//******************************************************************************

// Converte uma imagem em escala de cinza
void  MyImage_GrayScale(SDL_Renderer *renderer, MyImage *image)
{
  SDL_Log(">>> MyImage_GrayScale()");
    if (!renderer)
  {
    SDL_Log("\t*** Erro: Renderer inválido (renderer == NULL).");
    SDL_Log("<<< MyImage_GrayScale()");
    return;
  }

  if (!image || !image->surface)
  {
    SDL_Log("\t*** Erro: Imagem inválida (image == NULL ou image->surface == NULL).");
    SDL_Log("<<< MyImage_GrayScale()");
    return;
  }

  // Trava a superficie e altera o cursor para hourglass
  SDL_Log("\tConvertendo Imagem para escala de cinza.");
  SDL_SetCursor(hourglassMouseCursor);
  SDL_LockSurface(image->surface);

  const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(image->surface->format);
  const size_t pixelCount = image->surface->w * image->surface->h;

  Uint32 *pixels = (Uint32 *)image->surface->pixels;
  Uint8 r = 0;
  Uint8 g = 0;
  Uint8 b = 0;
  Uint8 a = 0;
  Uint8 Y = 0;

  //𝑌=0.2125∗𝑅+0.7154∗𝐺+0.0721∗𝐵

  /*Para cada pixel da superficie, aplica a formula acima e altera seu valor
  para o resultado */
  for (size_t i = 0; i < pixelCount; ++i)
  {
    SDL_GetRGBA(pixels[i], format, NULL, &r, &g, &b, &a);

    Y = .2125 * r + 0.7154 * g + 0.0721 * b;

    pixels[i] = SDL_MapRGBA(format, NULL, Y, Y, Y, a);
  }

  // Após manipularmos os pixels da superfície, liberamos a superfície.
  SDL_UnlockSurface(image->surface);

  // Atualizamos a textura da imagem
  SDL_DestroyTexture(image->texture);
  image->texture = SDL_CreateTextureFromSurface(renderer, image->surface);
  
  SDL_Log("\tConversao para escala de cinza finalizada");
  SDL_SetCursor(defaultMouseCursor);

  SDL_Log("<<< MyImage_GrayScale()");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

static bool MyImage_IsGrayScale(MyImage *image)
{
  SDL_Log(">>> MyImage_IsGrayScale()");
  if (!image || !image->surface)
  {
    SDL_Log("\t*** Erro: Imagem inválida (image == NULL ou image->surface == NULL).");
    SDL_Log("<<< MyImage_IsGrayScale(MyImage *image)()");
    return false;
  }

  SDL_LockSurface(image->surface);

  const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(image->surface->format);
  const size_t pixelCount = image->surface->w * image->surface->h;

  Uint32 *pixels = (Uint32 *)image->surface->pixels;
  Uint8 r = 0;
  Uint8 g = 0;
  Uint8 b = 0;
  Uint8 a = 0;

  /*Para cada pixel da superficie, verifica se os valores r,g r b sao iguais*/
  for (size_t i = 0; i < pixelCount; ++i)
  {
    SDL_GetRGBA(pixels[i], format, NULL, &r, &g, &b, &a);
    
    if (r != g || g!= b) {
      return false;
    }
  }
  SDL_UnlockSurface(image->surface);
  //Se todos os pixels tem valores r, g e b iguais, retorna true
  return true;
}

//******************************************************************************
// ** FIM IMPLEMENTACOES DO PROJETO 1
//******************************************************************************

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  atexit(shutdown);

  //Obter caminho da imagem
  if (argc < 2) {
      printf("Por favor escreva o caminho da imagem: %s <caminho_da_imagem.ext>\n", argv[0]);
      return 1;
  }
  const char *caminho_da_imagem = argv[1];

  if (initialize() == SDL_APP_FAILURE)
    return SDL_APP_FAILURE;

  if (!load_rgba32(caminho_da_imagem, g_window.renderer, &g_image))
    return SDL_APP_FAILURE;

  return 0;
}