Através de entrada Serial UART, o sistema deve interagir com o simulador de
sistema de elevadores, atendendo os seguintes requisitos:

# Requisitos funcionais

## O sistema

- Gerenciar 3 elevadores simultaneamente
- Cada elevador é independente e atende somente às requisições de sua coluna
- Cada elevador possui um identificador único
- Em caso de múltiplos comandos acionados a um mesmo elevador, o sistema deve priorizar, nesta ordem:
  - Manter a direção de movimento atual (subindo, descendo)
  - Completar ordens atuais dos botões internos
  - Distância do elevador até o comando externo

## Limites

- Não deixar elevador subir mais do que 15 andares
- Não deixar elevador descer abaixo do andar 0 (térreo)

## Comandos vindos do simulador

- Comando de subida/descida de elevador por botão externo
- Comando do andar desejado pelos botões internos dos elevadores
- Notificação quando elevador atinge a marca de cada andar

## Ações do controlador

- Inicializar cada elevador, abrindo portas no andar 0
- Ao atingir um andar desejado, parar o elevador no ponto exato do alinhamento do andar
- Após parar o elevador em um andar desejado, abrir a porta e esperar 1 segundo
- Quando abrir a porta, apagar todas as luzes acesas nos botões correspondentes ao andar, direção e elevador atual
- Ao receber um comando por botão interno, acender a luz deste botão
- Durante percurso de subida e descida, atender chamadas de acordo com ordem de prioridades
- Antes de qualquer início de movimentação, garantir que a porta está fechada e esperar 1 segundo
- Caso não haja nenhum comando acionado para o elevador, fechar as portas após 5 segundos e manter-se no mesmo andar
- Consultar altura dos elevadores periodicamente para verificar se o sistema está funcionando corretamente

# Requisitos não funcionais

- Linguagem C
- Placa Tiva C-Series TM4C1294XL
- Utilização da biblioteca TivaWare
- IDE IAR 9
- RTOS Keil RTX5
- Atender às [especificações do simulador](../docs/simulator_spec.pdf)
- Compilado em sistema operacional Windows
- Simulador rodando em sistema operacional Windows
- Comunicação via Serial UART
- Código versionado com git e publicação no GitHub
