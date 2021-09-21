# Capitolo 1
### Linguaggi, Macchine Astratte, Teoria della Computabilità

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