# Projeto1CompVisual
## Grupo:

Gabriel Shihao Chen Yin - 10408981 

Enrico Najjar Galdeano - 10402924 

Samuel Lopes Pereira - 10403767 

## O que é o projeto

Este projeto é um visualizador de imagens com ferramentas de análise e processamento. Ele carrega uma imagem, a converte para escala de cinza, gera e exibe seu histograma, e permite a equalização do histograma com um clique. Uma janela secundária mostra as análises da imagem, como a média de intensidade e o desvio padrão, juntamente com um botão para alternar entre as versões original e equalizada da imagem.
________________________________________

## Novas Estruturas e Variáveis Globais

•	Button: Uma nova struct foi criada para representar o botão de alternância, contendo suas dimensões (rect) e texto.

•	Cores do Botão: Cores foram definidas para os três estados do botão: neutro (NEUTRAL_COLOR), com o mouse por cima (HOVER_COLOR) e clicado (CLICKED_COLOR).

•	INFO_HEIGHT: Uma constante que define a altura da área de informações na janela filha, onde o texto e o botão são exibidos.

### Variáveis globais adicionadas:

o	histograma: Um array para armazenar o histograma da imagem original em escala de cinza.

o	histograma_equalizado: Um array que armazena o histograma após o processo de equalização.

o	surface_equalizada: Uma superfície criada a partir da equalização do histograma, contendo os pixels da imagem processada.

o	fonte: A fonte do texto utilizada na janela filha.

o	equalizado: Uma variável booleana que controla qual estado da imagem e do histograma é exibido. true indica a versão equalizada, e false a original. Esta variável guia a função de renderização.

o	mouseNoBotao e botaoClicado: Variáveis que rastreiam o estado do mouse sobre o botão, controlando a sua cor.

________________________________________

## Funções Modificadas

•	load_rgba32(): Modificada para receber um caminho de arquivo em vez de apenas o nome, e para verificar se a extensão é válida.

•	initialize(): Agora inicializa a biblioteca TTF (para fontes) e a janela secundária do histograma.

•	shutdown(): Responsável por finalizar a TTF e destruir a janela filha.

•	render(): Esta função renderiza todos os elementos visuais. Ela verifica a variável equalizado para decidir se deve exibir a imagem e o histograma originais ou equalizados. Em seguida, desenha o histograma, exibe as informações da imagem e, por fim, desenha o botão com a cor apropriada, baseada no estado das variáveis mouseNoBotao e botaoClicado.

________________________________________

## Funções Adicionadas

•	MyImage_GrayScale(): Converte a imagem para escala de cinza. Para cada pixel, aplica a fórmula $Y = 0.2125*R + 0.7154*G + 0.0721*B$ e atualiza os valores de R, G e B do pixel para Y.

•	Gerar_Histograma(): Preenche o array histograma. Ele itera sobre cada pixel da imagem, obtém sua intensidade e incrementa o nível correspondente no histograma.

•	Desenhar_Histograma(): Desenha o histograma na janela filha. Primeiro, encontra o valor máximo para normalizar a escala, garantindo que o pico mais alto chegue ao topo da janela. Em seguida, desenha uma barra vertical para cada nível de intensidade.

•	Exibir_Texto(): Exibe a média de intensidade e o desvio padrão. Classifica a imagem como clara, média ou escura e com contraste baixo, médio ou alto. As informações são formatadas em duas linhas e renderizadas na área de informações.
•	Analisar_Histograma(): Calcula a média de intensidade e o desvio padrão de um histograma, aplicando as fórmulas estatísticas.

•	Equalizar(): Equaliza o histograma e cria uma nova textura para a imagem. Primeiro, inicializa histograma_equalizado e obtém o total de pixels. Em seguida, preenche um array de mapeamento, que associa cada intensidade a um novo valor equalizado. Por fim, cria a surface_equalizada, mapeia os pixels da superfície original para a nova intensidade e atualiza a textura da imagem principal.

•	DesenharBotao(): Desenha o botão na janela filha. O texto do botão muda para "Equalizado" ou "Original" dependendo da variável equalizado. O tamanho do botão é definido com base no tamanho do texto, e ele é posicionado na área de informações.
•	MouseNoBotao(): Verifica se o cursor está sobre o botão. A função obtém as coordenadas globais do mouse e da janela filha para determinar se o clique ocorreu dentro da área do botão.

•	Salvar_Imagem(): Salva a imagem atualmente exibida. A função verifica o estado da variável equalizado para salvar a superfície correta (a original ou a equalizada) em um arquivo chamado "output_image.png".

________________________________________

## Loop Principal e main()

###	loop():

o	Gerencia os eventos do usuário, como fechar janelas, mover o mouse e cliques.

o	Quando o mouse se move, verifica se ele está sobre o botão e altera o estado correspondente.

o	Um "clique completo" (pressionar e soltar o botão esquerdo sobre o botão) alterna a variável equalizado.

o	Pressionar a tecla s salva a imagem atual.

o	A renderização da tela ocorre apenas quando há uma mudança no estado que exige uma atualização visual.

###	main():

o	Inicia o programa recebendo o caminho da imagem como um argumento.

o	Carrega a imagem com load_rgba32().

o	Ajusta o tamanho da janela principal para se adequar à imagem e a centraliza.

o	Posiciona a janela filha ao lado da janela principal.

o	Verifica se a imagem já está em escala de cinza e a converte se necessário.

o	Gera o histograma da imagem original.

o	Inicia o loop principal do programa.

________________________________________

# Instruções para Compilação e Execução
Para compilar e executar o projeto, siga os passos abaixo:

**1.Clone o repositório do GitHub.**

**2. Instale as bibliotecas necessárias**: Certifique-se de que as bibliotecas SDL3, SDL3_image e SDL3_ttf estão instaladas em seu sistema.

**3. Ajuste o Makefile**: Abra o arquivo Makefile e altere a variável SDL_DIR para o caminho onde a biblioteca SDL3 está instalada em sua máquina.

**4. Acesse a pasta src**: No terminal, navegue até a pasta src do projeto.

**5. Compile o código**: Digite mingw32-make para iniciar a compilação.

**6. Execute o programa**: Após a compilação, execute o programa usando o comando ./main <caminho_da_imagem>. Por exemplo: ./main "imagem.jpg".

	
	

