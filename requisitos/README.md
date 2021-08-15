# Requisitos do sistema

## Entidades:

- **Elevador:** Elevador individual. Contém porta, botões externos em cada andar
  e botão interno para cada andar

- **Controlador:** Software que interage com o simulador e gerencia os elevadores

- **Simulador:** Simulação do sistema. Contém os elevadores, envia sinais e recebe comandos do simulador

## Nomenclatura:

- **Sinal:** Informação produzida pelo simulador e recebida pelo controlador

- **Comando:** Informação produzida pelo sistema, e recebida pelo simulador

- **Requisição:** Quando um elevador é requisitado para atender um andar

- **Direção:** Direção de movimento do elevador. Subir/Descer

# Requisitos funcionais

## O sistema

### RF1: O controlador deve interagir com o simulador através de Serial UART, atendendo as [especificações do simulador](../docs/simulator_spec.pdf)

### RF2: O controlador deve gerenciar 3 elevadores simultaneamente

- RF2.1: Cada elevador deve atender somente às requisições de sua coluna

### RF3: O controlador deve priorizar, em caso de múltiplas requisições a um mesmo elevador:

- RF3.1: O elevador deve manter a direção de movimento atual (subindo, descendo)
- RF3.2: O elevador deve atender requisições pendentes dos botões internos
- RF3.3: O elevador deve atender a requisição mais perta
- RF3.4: Em caso de empate, o elevador deve atender a requisição abaixo do seu andar

### RF4: O controlador deve respeitar os limites da simulação

- RF4.1: O controlador não deve permitir um elevador subir mais do que 15 andares
- RF4.2: O controlador não deve permitir um elevador descer abaixo do andar 0 (térreo)

## Ações do controlador

### RF5: Controlador deve esperar sinal do simulador indicando que está inicializado

- RF5.1: Controlador deve inicializar cada elevador, abrindo portas no andar 0, após o sinal do simulador

### RF6: Controlador deverá monitorar sinais de chamadas e atendê-las de acordo com ordem de prioridades (RF3)

### RF7: Controlador deve parar o elevador no ponto exato do alinhamento do andar quando atingir andar requisitado

### RF8: Controlador deverá controlar abrimento e fechamento de portas dos elevadores

- RF8.1: O elevador deve abrir a porta após parar o elevador em um andar requisitado
- RF8.2: Elevador deverá estar com portas fechadas antes de qualquer início de movimentação
- RF8.3: Controlador deve esperar até fechamento completo antes de comandar movimentação
- RF8.4: Elevador deve fechar as portas após 5 segundos e manter-se parado caso não haja nenhuma requisição pendente

### RF9: Controlador deve controlar luzes dos botões internos dos elevadores

- RF9.1: Elevador deverá acender luz do botão interno quando receber sinal de requisição interna
- RF9.2: Elevador deverá apagar todas as luzes acesas nos botões correspondentes ao andar, direção e elevador atual, quando atingir um andar requisitado

# Requisitos não funcionais

### RNF1: Hardware do sistema deve ser baseado no microcontrolador TM4C1294

### RNF2: Software deve ser desenvolvido em linguagem C

### RNF3: Software Utilização da biblioteca TivaWare

### RNF4: Software deve utilizar o sistema operacional RTOS Keil RTX5

### RNF5: O ambiente de desenvolvimento do software deve ser o IAR EWARM versão 9
