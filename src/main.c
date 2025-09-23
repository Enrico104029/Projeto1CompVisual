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

// Array para armazenar intensidades
int histograma[256];

//fonte do texto
int histograma_equalizado[256];

//Superficie para armazenar pixels equalizados
SDL_Surface *surface_equalizada = NULL;

//Fonte do texto
TTF_Font* fonte;

//Variável que armazena se histograma esta equalizado
bool equalizado = false;

//Variaveis que armazenam o estado do botao
int mouseNoBotao = false;
int botaoClicado = false;

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

//Preenche um array(histograma) com quantidade de pixeis em cada nivel de intensidade 
//baseado em uma superficie
static bool Gerar_Histograma(SDL_Surface *surface, int* histograma);

//Desenha histograma
void Desenhar_Histograma(SDL_Renderer* renderer, const int* histograma, int width, int height);

// Devolve a media de intensidade e o desvio padrao do histograma
void Analisar_Histograma(const int* histograma, int* media_intensidade, double* desvio_padrao);

//Exibe media de intensidade , desvio padrao e suas classificacoes na tela
void Exibir_Texto(SDL_Renderer* renderer, int media_intensidade, double desvio_padrao);

//Preenche "histograma_equalizado e atualiza a textura da imagem"
void Equalizar(MyImage* image, int* histograma );

//Desenha o Botao com base no estado da imagem e com a cor passado no parametro
void DesenharBotao(SDL_Renderer* renderer, Button* button, SDL_Color buttonColor);

//Verifica se mouse esta em cima do botao
bool MouseNoBotao();


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MyWindow_initialize(MyWindow *window, const char *title, int width, int height, SDL_WindowFlags window_flags)
{
  SDL_Log("\tMyWindow_initialize(%s, %d, %d)", title, width, height);

  if (!window)
  {
    SDL_Log("\t\t*** Erro: Janela/renderizador inválidos (window == NULL).");
    return false;
  }

  return SDL_CreateWindowAndRenderer(title, width, height, window_flags, &window->window, &window->renderer);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyWindow_destroy(MyWindow *window)
{
  SDL_Log(">>> MyWindow_destroy()");

  if (!window)
  {
    SDL_Log("\t*** Erro: Janela/renderizador inválidos (window == NULL).");
    SDL_Log("<<< MyWindow_destroy()");
    return;
  }

  SDL_Log("\tDestruindo MyWindow->renderer...");
  SDL_DestroyRenderer(window->renderer);
  window->renderer = NULL;

  SDL_Log("\tDestruindo MyWindow->window...");
  SDL_DestroyWindow(window->window);
  window->window = NULL;

  SDL_Log("<<< MyWindow_destroy()");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyImage_destroy(MyImage *image)
{
  SDL_Log(">>> MyImage_destroy()");

  if (!image)
  {
    SDL_Log("\t*** Erro: Imagem inválida (image == NULL).");
    SDL_Log("<<< MyImage_destroy()");
    return;
  }

  if (image->texture)
  {
    SDL_Log("\tDestruindo MyImage->texture...");
    SDL_DestroyTexture(image->texture);
    image->texture = NULL;
  }

  if (image->surface)
  {
    SDL_Log("\tDestruindo MyImage->surface...");
    SDL_DestroySurface(image->surface);
    image->surface = NULL;
  }

  SDL_Log("\tRedefinindo MyImage->rect...");
  image->rect.x = image->rect.y = image->rect.w = image->rect.h = 0.0f;

  SDL_Log("<<< MyImage_destroy()");
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
bool MyImage_update_texture_with_surface(MyImage* image, SDL_Renderer *renderer, SDL_Surface *surface)
{
  SDL_Log(">>> MyImage_update_texture_with_surface()");

  if (!image)
  {
    SDL_Log("\t*** Erro: Imagem inválida (image == NULL).");
    SDL_Log("<<< MyImage_update_texture_with_surface()");
    return false;
  }

  if (!renderer)
  {
    SDL_Log("\t*** Erro: Renderer inválido (renderer == NULL).");
    SDL_Log("<<< MyImage_update_texture_with_surface()");
    return false;
  }

  if (!surface)
  {
    SDL_Log("\t*** Erro: Superfície inválida (surface == NULL).");
    SDL_Log("<<< MyImage_update_texture_with_surface()");
    return false;
  }

  SDL_DestroyTexture(image->texture);

  image->texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (!image->texture)
  {
    SDL_Log("\t*** Erro ao criar textura: %s", SDL_GetError());
    SDL_Log("<<< MyImage_update_texture_with_surface()");
    return false;
  }

  SDL_Log("\tObtendo dimensões da textura...");
  SDL_GetTextureSize(image->texture, &image->rect.w, &image->rect.h);

  SDL_Log("<<< MyImage_update_texture_with_surface()");
  return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MyImage_restore_texture(MyImage* image, SDL_Renderer *renderer)
{
  SDL_Log(">>> MyImage_restore_texture()");
  
  if (!MyImage_update_texture_with_surface(image, renderer, image->surface))
  {
    SDL_Log("\t*** Erro ao restaurar a textura da imagem.");
    return false;
  }

  SDL_Log("<<< MyImage_restore_texture()");
  return true;  
}

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

  SDL_Log("\tCriando janela e renderizador...");
  if (!MyWindow_initialize(&g_window, WINDOW_TITLE, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, 0))
  {
    SDL_Log("\t*** Erro ao criar a janela e/ou renderizador: %s", SDL_GetError());
    SDL_Log("<<< initialize()");
    return SDL_APP_FAILURE;
  }

  //Criar janela filha
  SDL_Log("\tCriando janela filha e renderizador...");
  if (!MyWindow_initialize(&child_window, CHILD_WINDOW_TITLE, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, 0))
  {
    SDL_Log("\t*** Erro ao criar a janela e/ou renderizador: %s", SDL_GetError());
    SDL_Log("<<< initialize()");
    return SDL_APP_FAILURE;
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

  SDL_Log("Destruindo superfície extra ...");
  SDL_DestroySurface(surface_equalizada);
  surface_equalizada = NULL;

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

//Renderiza imagem e janela filha com base na variavel "equalizado"
void render(void)
{
  SDL_Log(">>> render()");
  //Janela Principal
  SDL_SetRenderDrawColor(g_window.renderer, 20,20,20, 255);
  SDL_RenderClear(g_window.renderer);

  // Janela Filha
  SDL_SetRenderDrawColor(child_window.renderer, 20, 20, 20, 255);
  SDL_RenderClear(child_window.renderer);

  int media;
  double desvio;

  // Equalizar ou restaurar imagem com base na variavel
  if (equalizado) {
    Equalizar(&g_image, histograma);
    Desenhar_Histograma(child_window.renderer, histograma_equalizado, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT- INFO_HEIGHT);
    Analisar_Histograma(histograma_equalizado, &media, &desvio);
  } else {
    MyImage_restore_texture(&g_image, g_window.renderer);
    Desenhar_Histograma(child_window.renderer, histograma, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT- INFO_HEIGHT);
    Analisar_Histograma(histograma, &media, &desvio);;
  }

  //Exibir informaoes de de media e desvio padrao
  Exibir_Texto(child_window.renderer, media, desvio);

  //Renderizar textura da imagem
  SDL_RenderTexture(g_window.renderer, g_image.texture, &g_image.rect,&g_image.rect);

  // Desenhar o botao com base no seu estado atual
  if (mouseNoBotao) {
    if(botaoClicado){
      DesenharBotao(child_window.renderer, &button, CLICKED_COLOR);
    }
    else{
      DesenharBotao(child_window.renderer, &button, HOVER_COLOR);
    }
    
  } else  {
    DesenharBotao(child_window.renderer, &button, NEUTRAL_COLOR);
  }
 

  SDL_RenderPresent(g_window.renderer);
  SDL_RenderPresent(child_window.renderer);
  SDL_Log("<<< render()");
}


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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

//Preenche um array(histograma) com quantidade de pixeis em cada nivel de intensidade 
//baseado em uma superficie
static bool Gerar_Histograma(SDL_Surface *surface, int* histograma)
{
  SDL_Log(">>> Gerar_Histograma()");
  if (!surface)
  {
    SDL_Log("\t*** Erro: Superficie inválida (surface == NULL).");
    SDL_Log("<<< Gerar_Histograma()");
    return false;
  }

  // Inicializa o histograma 
  for (int i = 0; i < 256; i++) {
      histograma[i] = 0;
  }

  SDL_LockSurface(surface);
  const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(surface->format);
  const size_t pixelCount = surface->w * surface->h;

  Uint32 *pixels = (Uint32 *)surface->pixels;
  Uint8 r = 0;
  Uint8 g = 0;
  Uint8 b = 0;
  Uint8 a = 0;

  /*Para cada pixel da superficie, incrementa a intensidade correspondente no histograma*/
  for (size_t i = 0; i < pixelCount; ++i)
  {
    SDL_GetRGBA(pixels[i], format, NULL, &r, &g, &b, &a);
    
    histograma[r] ++ ;
  }

   SDL_UnlockSurface(surface);
   SDL_Log("<<< Gerar_Histograma()");
  //Se todos os pixels tem valores r, g e b iguais,
  return true;
}
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

//Desenha o histograma na janela filha
void Desenhar_Histograma(SDL_Renderer* renderer, const int* histograma_parametro, int width, int height)
{
  SDL_Log(">>> Desenhar_Histograma()");

  // Encontra o valor máximo para a escala
  int max_value = 0;
  for (int i = 0; i < 256; i++) {
      if (histograma_parametro[i] > max_value) {
          max_value = histograma_parametro[i];
      }
  }
  
  // Cor das barras do histograma (branco)
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  // Desenha uma barra para cada intensidade
  for (int i = 0; i < 256; i++) {

      // Obter altura da barra
      float altura_barra ;
      if (histograma_parametro[i] > 0)
      {
        altura_barra = (float)histograma_parametro[i] / max_value;
      }else{
        altura_barra = 0;
      }
      
      int altura_barra_pixels = (int)(altura_barra * height);

      //Posição e dimensões da barra
      int x = round(i * ((float)width / (float)256));
      int y1 = height;
      int y2 = height - altura_barra_pixels;

      // Desenha a linha
      SDL_RenderLine(renderer, x, y1, x, y2);
  }
  

  SDL_Log("<<< Desenhar_Histograma()");
}
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

void Exibir_Texto(SDL_Renderer* renderer, int media_intensidade, double desvio_padrao)
{
  SDL_Log(">>> Exibir_Texto()");
  SDL_Color textColor = {255, 255, 255, 255};

  char claridade[7]  ;
  //Classificar imagem como clara escura ou media
  if (media_intensidade <=85){
      strcpy(claridade, "Escura");
  }
  else{
    if(media_intensidade <=170){
      strcpy(claridade, "Media");
    }
    else{
      strcpy(claridade, "Clara");
    }
  }

  char contraste[7]  ;
  //Classificar imagem com contraste baixo, medio ou alto
  if (desvio_padrao <=50){
      strcpy(contraste, "Baixo");
  }
  else{
    if(desvio_padrao <=90){
      strcpy(contraste, "Medio");
    }
    else{
      strcpy(contraste, "Alto");
    }
  }

  //linhas de texto para exibir
  char linha1[100];
  char linha2[100];

  //formatar strings completas
  snprintf(linha1, 100, "Media intensidade: %d -> Imagem %s", media_intensidade, claridade);
  snprintf(linha2, 100, "Desvio Padrao: %.2f -> %s Contraste", desvio_padrao,contraste);

  //tamanhos
  size_t linha1_length = strlen(linha1);
  size_t linha2_length = strlen(linha2);

  /*
    Linha 1
  */
  SDL_Surface* textSurface = TTF_RenderText_Blended(fonte, linha1,linha1_length,textColor);

  if (!textSurface) {
        SDL_Log("Erro ao renderizar texto: ");
        return;
  }

  SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
  if (!textTexture) {
      SDL_Log("Erro ao criar textura de texto: %s", SDL_GetError());
      SDL_DestroySurface(textSurface);
      return;
  }
  
  SDL_DestroySurface(textSurface);

  //Posicionamento linha 1
  float linha1_w, linha1_h;
  SDL_FRect textRect;

  SDL_GetTextureSize(textTexture, &linha1_w, &linha1_h);
  textRect.w = linha1_w;
  textRect.h = linha1_h;
  textRect.x = 10;
  textRect.y = ((float)DEFAULT_WINDOW_HEIGHT- (INFO_HEIGHT/2)) - (linha1_h + 5);
  

  //Renderizar linha 1
  SDL_RenderTexture(renderer, textTexture, NULL, &textRect);

  /*
    Linha 2
  */
  textSurface = TTF_RenderText_Blended(fonte, linha2,linha2_length,textColor);

  if (!textSurface) {
        SDL_Log("Erro ao renderizar texto: ");
        return;
  }

  textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
  if (!textTexture) {
      SDL_Log("Erro ao criar textura de texto: %s", SDL_GetError());
      SDL_DestroySurface(textSurface);
      return;
  }
  
  SDL_DestroySurface(textSurface);

  //Posicionamento linha 2
  float linha2_w, linha2_h;

  SDL_GetTextureSize(textTexture, &linha2_w, &linha2_h);
  textRect.w = linha2_w;
  textRect.h = linha2_h;
  textRect.x = 10;
  textRect.y = textRect.y+ linha1_h + 10;


  //Desenho linha 2
  SDL_RenderTexture(renderer, textTexture, NULL, &textRect);

  SDL_Log("<<< Exibir_Texto()");
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

// Devolve a media de intensidade e o desvio padrao de um histograma
void Analisar_Histograma(const int* histograma, int* media_intensidade, double* desvio_padrao)
{
  SDL_Log(">>> Analisar_Histograma()");
  int total_pixels = 0;
  int total_intensidades = 0;

  //Calcular media
  for(int i = 0; i < 256; i++){
    total_pixels += histograma[i];
    total_intensidades += histograma[i]* i;
  }

  *media_intensidade = total_intensidades/ total_pixels;

  //Calcular desvio_padrao
  double somatoria;
    for (int i = 0; i < 256; ++i) {
        float diferenca = i - *media_intensidade;
        somatoria += (diferenca * diferenca) * histograma[i];
    }
  double variancia = somatoria / total_pixels;

  *desvio_padrao = SDL_sqrt(variancia);
 
  SDL_Log("<<< Analisar_Histograma()");
}
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

//Equaliza o histograma e atualiza a imagem
void Equalizar(MyImage* image, int* histograma )
{
  SDL_Log(">>> Equalizar()");

  //array para armazenar mapeamento de cada intensidade
  double mapeamento[256];

  //obter total de pixels e inicializar o histograma equalizado e mapeamento
  int total_pixels = 0;
  for(int i = 0; i < 256; i++){
    total_pixels += histograma[i];
    histograma_equalizado[i] = 0;
  }

  // Preencher tabela de mapeamento
  for(int i = 0; i < 256; i++){
    double prob_ocorr = (double)histograma[i] / total_pixels;
    if(i == 0){
      mapeamento[i] = 255 * prob_ocorr;
    }
    else{
      mapeamento[i] = mapeamento[i-1] + (255 * prob_ocorr);
    }

    histograma_equalizado[(int)round(mapeamento[i])] += histograma[i];

  }
    
  //Desenha histograma equalizado
  Desenhar_Histograma(child_window.renderer, histograma_equalizado, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT- INFO_HEIGHT);

  // Mapear pixels da superficie para superficie_equalizada
  const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(image->surface->format);

  //Crie superficie equalizada a partir do histograma
  surface_equalizada = SDL_CreateSurface(image->surface->w, image->surface->h, image->surface->format);
  if (!surface_equalizada){
    {
      SDL_Log("*** Erro: Superfície extra (equalizada) inválida!");
      return ;
    }
  }

  
  SDL_LockSurface(image->surface);
  SDL_LockSurface(surface_equalizada);

  const size_t pixelCount = image->surface->w * image->surface->h;

  Uint32 *pixels = (Uint32 *) image->surface->pixels;
  Uint32 *output = (Uint32 *) surface_equalizada->pixels;

  Uint8 r = 0;
  Uint8 g = 0;
  Uint8 b = 0;
  Uint8 a = 0;

  //Para cara pixel da imagem, trocar sua intensidade pela obtida
  // apos equalizar o histograma e salva no mapeamento
  for (size_t i = 0; i < pixelCount; ++i)
  {
    SDL_GetRGBA(pixels[i], format, NULL, &r, &g, &b, &a);

    int intensidade_alterada = (int)round(mapeamento[r]);
    output[i] = SDL_MapRGBA(format, NULL, intensidade_alterada, intensidade_alterada, intensidade_alterada, a);
  }

  SDL_UnlockSurface(image->surface);
  SDL_UnlockSurface(surface_equalizada);

  //Utiliza a nova superficie para atualizar a textura da imagem
  MyImage_update_texture_with_surface(&g_image,g_window.renderer, surface_equalizada);

  SDL_Log("<<< Equalizar()");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

//Desenha o Botao com base no estado da imagem e com a cor passado no parametro
void DesenharBotao(SDL_Renderer* renderer, Button* button, SDL_Color buttonColor)
{
  SDL_Color textColor = {255, 255, 255, 255};

  // Desenha o retângulo do botão
  SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);

  //Alterar texto com base na equalizacao
  if(equalizado){
    button->text = "Equalizado";
  }else{
    button->text = "Original";
  }

  // Desenha o texto do botão
  SDL_Surface* textSurface = TTF_RenderText_Blended(fonte, button->text, strlen(button->text), textColor);
  if (!textSurface) {
    SDL_Log("Erro ao renderizar texto do botao ");
    return;
  }

  SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
  if (!textTexture) {
      SDL_Log("Erro ao criar textura de texto: %s", SDL_GetError());
      SDL_DestroySurface(textSurface);
      return;
  }
  
  SDL_DestroySurface(textSurface);

  float text_w, text_h;
  SDL_GetTextureSize(textTexture,&text_w, &text_h);

  //Se o tamanho do botao ainda nao foi estabelecido,
  //definir tamanho do botao com tamanho do texto
  if(button->rect.w == 0) {
    button->rect.w = text_w + 60;
    button->rect.h = text_h + 40;
    button->rect.x = DEFAULT_WINDOW_WIDTH - button->rect.w - 20;
    button->rect.y = DEFAULT_WINDOW_HEIGHT - (INFO_HEIGHT / 2) - (button->rect.h/2);
  }

  //Retangulo de posicao do texto
  SDL_FRect textRect = {
      (float)button->rect.x + (button->rect.w - text_w) / 2,
      (float)button->rect.y + (button->rect.h - text_h) / 2,
      (float)text_w,
      (float)text_h
  };

  // Desenhar botao e texto
  SDL_RenderFillRect(renderer, &button->rect);
  SDL_RenderTexture(renderer, textTexture, NULL, &textRect);

  SDL_DestroyTexture(textTexture);
  
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

//Verifica se mouse esta em cima do botao
bool MouseNoBotao()
{
    // Pega as coordenadas globais do mouse 
    float mouse_x, mouse_y;
    SDL_GetGlobalMouseState(&mouse_x, &mouse_y);

    // Obtém a posição da janela filha na tela
    int child_x, child_y;
    SDL_GetWindowPosition(child_window.window, &child_x, &child_y);

    // Converte as coordenadas do botão (relativas à janela filha) para coordenadas da tela
    int button_x_global = button.rect.x + child_x;
    int button_y_global = button.rect.y + child_y;
    int button_w = button.rect.w;
    int button_h = button.rect.h;

    // Realiza a verificação usando as coordenadas globais
    return (mouse_x >= button_x_global && mouse_x <= button_x_global + button_w &&
            mouse_y >= button_y_global && mouse_y <= button_y_global + button_h);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

//Salva a imagem atual
void Salvar_Imagem()
{
  SDL_Log(">>> Salvar_Imagem()");
  SDL_Surface* superficie = NULL;

  // Verifica qual superfície deve ser salva com base na variável 'equalizado'
  if (equalizado) {
      superficie = surface_equalizada;
  } else {
      superficie = g_image.surface;
  }

  // Verifica se a superficie é valida
  if (superficie == NULL) {
      SDL_Log("Erro: Nenhuma superfície de imagem válida para salvar.");
      SDL_Log("<<< Salvar_Imagem()");
      return;
  }
  
  // Salva a superficie no arquivo,sobrescreve se ja existir

  IMG_SavePNG(superficie, "output_image.png");
  SDL_Log("<<< Salvar_Imagem()");
}


//******************************************************************************
// ** FIM IMPLEMENTACOES DO PROJETO 1
//******************************************************************************


static void loop(void)
{
  SDL_Log(">>> loop()");

  render(); 

  SDL_Event event;
  bool isRunning = true;
  while (isRunning)
  {
    bool must_refresh = false;
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_EVENT_QUIT:
          isRunning = false;
          break;

        //Ao clicar para fechar qualquer janela, encerrar programa
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
          SDL_Window *closed_window = SDL_GetWindowFromID(event.window.windowID);
          if (closed_window == g_window.window || closed_window == child_window.window)
          {
            isRunning = false;
          }
          break;

        //Ao mexer o mouse
        case SDL_EVENT_MOUSE_MOTION:
          // Checar se estado do botao deve mudar
          if (MouseNoBotao() != mouseNoBotao) {
            must_refresh = true;
          }
          mouseNoBotao = MouseNoBotao();

          // Se mouse nao estiver no botao, nao esta clicando
          if(!mouseNoBotao){
            botaoClicado = false;
          }
          break;
        
        //Ao clicar na tela
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
          if (event.button.button == SDL_BUTTON_LEFT) {
           
            if (MouseNoBotao( )) {
             
              botaoClicado = true;
              must_refresh = true;
            } 
          }
          break;

        //Ao parar de clicar
        case SDL_EVENT_MOUSE_BUTTON_UP:
          if (event.button.button == SDL_BUTTON_LEFT) 
          {
             //Se mouse estiver no botao = click = trocar variavel equalizado
             if (MouseNoBotao( )) {
              if (equalizado) {
                  equalizado = false;
                } else {
                  equalizado = true;
                }

                botaoClicado = false;
                must_refresh = true;
             }
          }
          break;

   
      }
    }
    if(must_refresh){
       render();
    }
  }
  
  SDL_Log("<<< loop()");
}

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

  SDL_Log("Criando cursores do mouse...");
  defaultMouseCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
  hourglassMouseCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
  SDL_SetCursor(defaultMouseCursor);

  // Altera tamanho da janela se a imagem for maior do que o tamanho padrão
  int imageWidth = (int)g_image.rect.w;
  int imageHeight = (int)g_image.rect.h;

  SDL_SetWindowSize(g_window.window, imageWidth, imageHeight);


  // Coloca a janela principal no centro
  SDL_SetWindowPosition(g_window.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_SyncWindow(g_window.window);


  /*
    Colocar a janela filha ao lado da janela pai
  */
  int x, y;
  int w, h ;

  // Pegar a posicao e tamanho da janela pai
  SDL_GetWindowPosition(g_window.window, &x, &y);
  SDL_GetWindowSize(g_window.window, &w, &h);
 
  SDL_SetWindowPosition(child_window.window, x + w +20, SDL_WINDOWPOS_CENTERED);

  //Verifica se imagem esta em escala de cinza,
  //caso nao esteja, ela é convertida
  if(!MyImage_IsGrayScale(&g_image))
  {
    MyImage_GrayScale(g_window.renderer, &g_image);
  }
  
  // Criar histograma da imagem
  Gerar_Histograma(g_image.surface, histograma);

  //Exibir botao
  loop();

  return 0;
}