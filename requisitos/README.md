# Requisitos do sistema

## Entidades:

- **Elevador:** Elevador individual. Contém porta, botões externos para subida e descida
  em cada andar e botão interno para cada andar

- **Controlador:** Software que interage com o _simulador_ e gerencia os _elevadores_

- **Simulador:** Simulação do sistema. Contém os _elevadores_, envia sinais e recebe comandos do _simulador_

- **Usuário:** Programador, responsável por iniciar a simulação, gravar o software no dispositivo
  e configurar a comunicacão entre eles

## Nomenclatura:

- **Sinal:** Informação produzida pelo _simulador_ e recebida pelo _controlador_

- **Comando:** Informação produzida pelo sistema, e recebida pelo _simulador_

- **Requisição:** Quando um _elevador_ é requisitado para atender um andar

- **Direção:** Direção de movimento do _elevador_. Subir/Descer

# Requisitos funcionais

## Interface

### RF1: O _controlador_ deve interagir com o _simulador_ através de Serial UART, atendendo as [especificações do _simulador_](../docs/simulator_spec.pdf)

### RF2: Após o início da simulação, o _usuário_ deve interagir com o sistema somente através do _simulador_

### RF3: Através do _simulador_, o _usuário_ pode efetuar as operações:

- RF1.1: _Requisitar_ um _elevador_ específico em um andar, com direção de subida ou descida
- RF1.2: _Requisitar_ deslocamento para um andar, através de botão interno no _elevador_

## Funcionamento dos elevadores

### RF4: Um _elevador_ deverá atender _requisições_ externas, embarcando passageiros nos andares requisitados

### RF5: Um _elevador_ deverá atender _requisições_ internas, desembarcando passageiros nos andares selecionados

### RF6: _Elevador_ deve respeitar limites do sistema

- RF6.1: _Elevador_ não devem subir acima do 15º andar
- RF6.2: _Elevador_ não devem descer abaixo do andar 0 (térreo)

### RF7: As portas do _elevador_ devem ser abertas para permitir embarque e desembarque de passageiros

- RF7.1: Portas devem ficar abertas por pelo menos 5 segundos

### RF8: _Elevador_ só poderá se movimentar com portas fechadas

### RF9: _Elevador_ deve fechar as portas após 5 segundos e manter-se parado caso não haja nenhuma _requisição_ pendente

### RF10: _Elevador_ deverá acender luz do botão interno quando receber _sinal_ de _requisição_ interna

### RF11: _Elevador_ deverá apagar todas as luzes acesas nos botões correspondentes ao andar quando atingir um andar requisitado

# Requisitos não funcionais

### RNF1: O _controlador_ deve gerenciar 3 _elevadores_ simultaneamente

### RNF2: Cada _elevador_ deve atender somente às _requisições_ de sua coluna

### RNF3: _Controlador_ deve buscar parar o _elevador_ no ponto exato do alinhamento do andar quando atingir andar requisitado

### RNF4: O _controlador_ deve priorizar, em caso de múltiplas _requisições_ a um mesmo _elevador_:

- RF3.1: O _elevador_ deve manter a direção de movimento atual (subindo, descendo)
- RF3.2: O _elevador_ deve atender _requisições_ pendentes dos botões internos
- RF3.3: O _elevador_ deve atender a _requisição_ mais perta
- RF3.4: Em caso de empate, o _elevador_ deve atender a _requisição_ abaixo do seu andar

### RNF5: Um _elevador_ não precisa parar em um andar com _requisição_ externa caso esteja indo no sentido contrário da _requisição_

### RNF6: O sistema deve informar o _usuário_ sobre erros através de uma interface independente da UART

### RNF7: A comunicação do _controlador_ com o _simulador_ deve ser monitorável

### RNF8: Todos os _elevadores_ não devem apresentar falhas no atendimento de pelo menos 10 _requisições_

### RNF9: Um _elevador_ deve ser capaz de atender uma sobrecarga de _requisições_ (requisições a todos os botões possíveis)

### RNF10: Caso o _elevador_ esteja parado, deve começar a atender uma requisição nova em menos de 1 segundo

# Restrições

### R1: A comunicação se dará com baud rate de 115200bps

### R2: Hardware do sistema deve ser baseado no microcontrolador TM4C1294

### R3: Software deve ser desenvolvido em linguagem C

### R4: Software deve utilizar a biblioteca TivaWare

### R5: Software deve ser desenvolvido utilizando objetos do sistema operacional RTOS Keil RTX5

### R6: O ambiente de desenvolvimento do software deve ser o IAR EWARM versão 9

### R7: Software não deve utilizar variáveis e funções globais

### R8: Cada parte do sistema deve ser desenvolvida em um arquivo separado, com arquivos de cabeçalho e de implementação

### R9: O sistema deve fazer uso de filas de mensagens

### R10: O _controlador_ deve ser multi-threaded, com uma thread dedicada por _elevador_

### R11: O sistema não precisa ser expansível para mais de 3 _elevadores_

### R12: O sistema não precisa ser expansível para mais de 15 andares
