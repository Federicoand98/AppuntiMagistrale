# Virtualizzazione
### 21/09/21



Virtualizzare un sistema (hardware e software) significa presentare all'uilizzatore una visione 
delle risorse del sistema diversa da quella reale.

Ciò è possibile introducendo un **livello di indirezione** tra la vista logica e quella fisica delle 
risorse.

Quindi l'obiettivo della virtualizzazione è quello di disaccoppiare il comportamento delle risorse
di un calcolatore dalla loro realizzazione fisica. Quindi apparendo diverse da quelle effettive 
della macchina.

Il software che si occupa di virtualizzare in parole semplici divide le risorse reali nel numero di 
macchine virtuali necessarie. Quindi ogni macchina virtuale avrà la sua CPU, GPU, RAM, ecc...

**Virtualizzazione a livello di processo**: i sistemi multitasking permettono l'esecuzione 
contemporanea di più processi, ognuno dei quale dispone di una macchina virtuale dedicata. Questo
tipo di virtualizzazione viene realizzata dal kernel del sistema operativo.

**Virtualizzazione della memoria**: in presenza di memoria virtuale, ogni processo vede uno spazio
di indirizzamento di dimensioni indipendenti dallo spazio fisico effettivamente a dispozione.
Anche questa virtualizzaione è realizzata dal kernel.

**Astrazione**: un oggetto astratto (risorsa virtuale) è la rappresentazione semplificata di un 
oggetto (risortsa fisica)

#### Virtualizzazione di un Sistema di Elaborazione
Tramite la virtualizzazione una singola piattaforma hardware viene condivisa da più elaboratori 
virtuali, ognuno gestito da un proprio sistema operativo.

Il disaccoppiamento viene realizzato dal **Virtual Machine Monitor (VMM)**, il cui compito
è quello di consentire la condivisione da parte di più macchine virtuali di una singola
piattaforma hardware.

Quindi il **VMM** è il **mediatore unico** nelle interazioni tra le macchine virtuali e
l'hardware, il quale garantisce: **isolamento tra le VM** e **stabilità del sistema**.


### Tecniche del VMM:
#### Emulazione
L'emulazione è l'insieme di tutti quei meccanismi che permettono l'esecuzione di un programma
compilato su un determiato sistema di girare su un qualsiasi altro sistema differente da quello
nel quale è stato compilato.
Quindi vengono emulate interamente le singole istruzioni dell'architettura ospitata.

I vantaggi dell'emulazione sono l'interoperabilità tra ambienti eterogenei, mentre gli svantaggi sono
le ripercussioni sulle performances.

Esistono principalmente due tecniche di emulazione: **interpretazione** e **ricompilazione dimanica**.

###### Interpretazione:
L'interpretazione si basa sulla lettura di ogni singola istruzione del codice macchina che deve
essere eseguito e sulla esecuzione di più istruzioni sull'host virtualizzante.

Produce un sovraccarico elevento in quanto potrebbero essere necessarie molte istruzioni dell'host
per interpretare una singola istruzione sorgente.

##### Compilazione dinamica
Invece di leggere una singola istruzione del sistema ospitato, legge interi blocchi di codice,
vengono analizzati, tradotti per la nuova architettura, ottimizzati e messi in esecuzione.

Il vantaggio in termini prestazionali rispetto all'interpretazione è notevolmente maggiore.
Ad esempio parti di codice utilizzati frequentemente vengono bufferizzati nella cache per evitare
di doverli ricompilare in seguito.