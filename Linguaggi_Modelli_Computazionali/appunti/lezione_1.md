# Capitolo 1
## Linguaggi, Macchine Astratte, Teoria della Computabilità

- **Algoritmo**: sequenza finita di mosse che risolve in un tempo finito una classe di problemi
- **Codifica**: descrizione dell'algoritmo tramite un insieme ordinato di frasi (istruzioni) di un linguaggio di programmazione, che specificano le azioni da svolgere
- **Programma**: testo scritto in accordo alla sintassi e alla semantica di un linguaggio di programmazione

**NB** un programma può non essere un algoritmo

Quindi, la'lgoritmo esprime la soluzione di un problema, un programma è la formmulazione di un algoritmo in un dato linguaggio di programmazione e l'esecuzione delle
azioni specificate dall'algoritmo, nell'ordine da esso specificato, a partire da dati in ingresso porta al risultato del problema.

#### Automa Esecutore
Quindi bisogna presupporre l'esistenza di un **Automa Esecutore**, ovvero una macchina astratta in grado di eseguire le azioni dell'algoritmo.

L'Automa Esecutore deve avere determiate caratteristiche:
- deve ricevere dall'esterno una descrizione dell'algoritmo
- deve essere capace di interpretare un linguaggio

Inoltre ha due vincoli di realizzabilità fisica:
- se l'automa è fatto di parti, esse sono in numero finito
- ingresso e uscita devono essere denotabili attraverso un insieme finito di simboli

Formalizzando cosa sia L'automa esecutore si arriva a definire il concetto di **computabilità**:
- **Approccio a gerarchia di macchine astratte**
- **Approccio Funzionale**
- **Sistema di riscrittura**

##### La Macchina Base
La macchina base è formalmente definita dalla tripla
$$<I, O, mfn>$$

dove:
- I = insieme finito dei simboli di ingresso
- O = insieme finito dei simboli di uscita
- mfn = I -> O (funzione di macchina)

Un esempio della macchina di base sono le porte logiche

I principali limiti della macchina base sono:
- risolvere i problemi con questo tipo di macchina 
- altro

##### Automa a stati finiti
L'Automa a stati finiti è il primo tentativo di introduzione del concetto di "memoria", è definita attraverso un automa con un numero finito di stati interni.
Un Automa a stati finiti è definito dalla quintupla:

$$<I, O, S, mfn, sfn>$$

dove:
- I = insieme finito dei simboli di ingresso
- O = insieme finito dei simboli di uscita
- S = insieme finito degli stati
- mfn = I x S -> O (fuznione di macchina)
- sfn = I x S -> S (funzione di stato)

I principali limiti dell'Automa a stati finiti:
- ca
- as
- sa

##### Macchina di Turing
Per superare il limite della memoria finita, viene introdotto un "nastro" come supporto di memorizzazione esterno.
La Macchina di Turing è definita dalla quintupla:

$$<A, S, mfn, sfn, dfn>$$

dove:
- A : insieme finito dei simboli di ingresso e uscita
- S : insieme finito degli stati (uno dei quali è HALT)
- mfn : A x S -> A (funzione di macchina)
- sfn : A x S -> S (funzione di stato)
- dfn : A x S -> D = {Left, Right, None} (funzione di direzione)

La memoria della Macchina di Turing è rappresentata da un nastro illimitatamente espandibile. La macchina, attraverso una testina, può leggere un simbolo dal nastro, 
scrivere sul nastro un simbolo specificato da mfn(), transitare in un nuovo stato interno specificato da sfn(), spostartsi sul nastro di una posizione nella direzione
indicata da dfn(). Quando la macchina arriva allo stato HALT, si arresta.

Risolvere un problema (programmare) con la macchina di turing significa:
- **definire la rappresentazione dei dati di partenza** da porre inizialmente sul nastro e di quelli di uscita che ci si aspetta di trovare sul nastro
- **definire il comportamento**, ossia le funzioni: mfn(), sfn(), dfn(), le quali sono concepite in modo da lasciare sul nastro la soluzione quando alla fine
la macchina si ferma.

Per definizione della Macchina di Turing, quando alla fine la macchina si ferma, sul nastro c'è la soluzione del problema, altrimenti c'è stato un errore di 
programmazione. (Comunque non è detto che SEMPRE arrivi ad HALT, attraverso le tre funzioni è possibile costruire un ciclo infinito).

# 23/09/21

Domande fondamentali:
- Esiste sempre una opportuna Macchina di Turing capace di risolvere qualunque problema?
- Esistono macchine più potetnti della Macchina di Turing?

**Tesi di Church-Turing**
Non esiste alcun formalismo capace di risolvere una classe di problemi più ampia di quella risolta dalla macchina di Turing.

###### Macchine Specifiche vs Macchine Universali
Una volta definita la parte di controllo, la MdT è in grado do risolvere un unico problema, quindi è **specifica** per quel tipo di problema.
Le macchine specifiche quindi sono convenienti ovviamente per usi specifici e per mercati di massa. Sono invece sconvenienti se si vuole una macchina di uso generale
con cui risolvere ogni problema. (Macchina Universale)

Fino ad ora l'algoritmo realizzato da una data MdT era cablato nella macchina. Se invece l'algoritmo viene scritto sul nastro, e la macchina lo legge si ottiene
una **Macchina di Turing Universale (UTM)**. In questo modo si ottiene una macchina il cui programma non cambia quando cambia il problema da risolvere. Quindi c'è
la necessità di un **LOADER**, un algoritmo cablato che legge dal nastro la descrizione dell'algoritmo.

Quindi richiede saper descrivere l'algoritmo richiesto, e per descrivere l'algoritmo serve un linguaggio e una macchina che lo interpreti. In questo modo possiamo
considerare la **UTM** come l'interprete di un linguaggio. E quindi modella il concetto di elaboratore di uso generale (fetch, decode, execute)

-- confronto mdt e von neumann --

##### Computazione e Interazione
Computazione e Interazione sono dimensioni ortogonali, espresse da due linguaggi distinti:
- **linguaggio di computazione**: definisce le primitive per esprimere l'elaborazione delle informazioni
- **linguaggio di coordinazione**: definisce le primitive per esprimere input/output di informazioni dal/verso il mondo esterno
- linguaggio di comunicazione: definisce quali informazioni saranno trasmesse mediante le primitive del linguaggio di coordinazione, nonchè il formato di tali informazioni

#### Introduzione alla Teoria della Computabilità
Secondo la tesi di Church-Turing non esiste un formalismo capace di risolvere una classe più ampia di problemi della MdT. Quindi se nemmeno la macchina di Turing
riesce a risolvere un determinato problema, **quel problema è irrisolubile**.

Definiamo quindi **PROBLEMA RISOLUBILE**: ovvero un problema la cui soluzione può essere espressa da una MdT (o formalismo equivalente).
Però la MdT cumputa **funzioni**, non problemi, perchè tutto torni bisogna associare a un problema una funzione.

Quindi, per instaurare questo legame definiamo la **funzione caratteristica di un problema**.

Dato un **problema P** e detti:
- **l'insieme X dei suoi dati di ingresso**
- **l'insieme Y delle risposte corrette**
si dice **funzione caratteristica del problema P** $$f_p:X -> Y$$

Bisogna quindi formalizzare quando una funzione caratteristica sia **computabile**

**FUNZIONE COMPUTABILE**
**Una funzione f:A->B è computabile se esiste una MdT che**:
    - data sul nastro una rappresentazione x \in A
**dopo un numero finito di passi**
    - produce sul nastro una rappresentazione di f(x) \in B

##### Funzioni Definibili vs Funzioni Computabili
Tutte le funzioni sono computabili? Oppure essistono invece funzioni definibili ma non computabili?
Occorre quindi confrontare le funzioni che possiamo **definire** con le funzioni che possiamo **computare** con la MdT.

D'ora in poi consideriamo solo le funzioni computabili su N, ovvero terminabili in un numero naturale finito di passi (può essere applicato anche il procedimento di Godel)

..
..
..
..
..
..
finite slide

# Capitolo 2
## Linguaggi e Grammatiche

Secondo il dizionario un linguaggio è: un insieme di parole e di metodi di combinazione delle parole usate e comprese da una comunità di persone.
Questa definizione è poco precisa in quanto non evita le ambiguità dei linguaggi naturali, non si presta a descrivere processi computazionali meccanizzabili e non aiuta a stabilire
proprietà.

Quindi c'è la necessita di una nozione di linguaggio più precisa. Un linguaggio come sistema formale, che permetta di definire:
- le frasi lecite
- se una frase appartiene ad un linguaggio o no
- come stabilire il significato di una frase
- quali elementi linguistici primitivi

**Sintassi**: l'insieme delle regole formali per la scrittura di programmi in un linguaggio, che dettano le modalità per costruire frasi corrette nel linguaggio stesso. la sintassi è
generalmente espressa tramite notazioni formali come BNF, EBNF, diagrammi sintattici

**Semantica**: l'insieme dei significati da attribuire alle frasi (sintatticamente corrette) costruite nel linguaggio. La semantica è esprimibile:
- a parole (poco precisa ed ambigua)
- mendiante azioni -> **semantica operazionale**
- mediante funzioni matematiche -> **semantica denotazionale**
- mediante formule logiche -> **semantica assiomatica**

Quindi possiamo definire la differenza tra interpretazione e compilazione:

Un **interprete** per un linguaggio L:
- accetta in ingresso le singole frasi di L
- le esegue volta per volta
Il risultato è la valutazione della frase

Un **compilatore** per un linguaggio L, invece:
- accetta in ingresso un intero programma scritto in L
- lo riscrive in un altro linguaggio più semplice
Il risultato è quindi una riscrittua della "macro-frase"

L'**analisi lessicale** consiste nella individuazione delle singole parole (token) di una frase. L'analizzatore lessicale detto scanner o lexer legge una sequenza di caratteri, li aggrega in 
token di opportune categorie (nomi, parole chiave, simboli, ecc).
L'**analisi sintattica** consiste nella verifica che la frase, intesa come sequenza di token, rispetti le regole grammaticali del linguaggio. L'analizzatore sintattico, detto parser, data
la sequena di token prodotta dallo scanner, genera una rappresentazione interna della frase, generalmente sotto forma di albero.
L'**analisi semantica** consiste nel determinare il significato di una frase. L'analizzatore semantica, data la rappresentazione prodotta dal parser, controlla la coerenza logica della frase.

##### Significato di una frase
Chiedersi quale sia il significato di una frase significa associare a quella frase un concetto nella nostra mente. Per fare ciò nella nostra mente deve esserci una funzione che associa
a ogni frase un concetto.
Quindi tale funzione deve dare significato:
- prima a ogni simbolo (carattere dell'alfabeto)
- poi a ogni parola (sequenza lecita di caratteri)
- infine a ogni frase (sequenza lecita di parole)

##### Definizioni
**Alfabeto**: un insieme finito e non vuoto di simboli atomici. es A = {a, b}

**Stringa**: una sequenza di simboli, ossia un elemento del prodotto cartesiano A^n. Di conseguenza si definisce:
    - Lunghezza di una stringa: il numero di simboli che la compongono
    - Stringa vuota: stringa di lunghezza zero.

**Linguaggio L su un alfabeto A**: un insieme di stringhe su A
    - Frase (sentence) di un linguaggio: una stringa appartenente a quel linguaggio
    - Cardinalità di un linguaggio: il numero delle frasi del linguaggio
        - linguaggio finito: ha cardinalità finita
        - linguaggio infinito: ha cardinalità infinita

**Chiusura A\* di un alfabeto A** (o linguaggio universale su A):
È l'insieme infinito di tutte le stringhe composte con sinboli di A: fomrula

**Chiusura positiva di A+ di un alfabeto A**:
È l'insieme infinito di tutte le stringhe non nulle composte con simboli di A: formula

#### Grammatica Formale
Una **grammatica** è una notazione formale con cui esprimere in modo rigoroso la **sintassi** di un linguaggio.
La grammatica è un quadrupla <VT, VN, P, S> dove:
- **VT**: è l'insieme finito di simboli terminali
- **VN**: è un insieme finito di simboli non terminali
- **P**: è un insieme finito di produzioni, ossia di regole di riscrittura **a -> b** dove **a** e **b** sono stringhe
- **S**: è un particolare simbolo non-terminale detto **simbolo iniziale** o **scopo** della grmmatica.

convenzioni

