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


..
..
..

### Realizzaione dell VMM
**Requisiti di Popek e Goldberg del 1974**

1. **Ambiente di esecuzione per i programmi sostanzialmente identico a quello della macchina reale**:
    Gli stessi programmi che eseguono nel sistema non virtualizzato possono essere eseguiti nelle VM
    senza modifiche e problemi.
2. **Garantire un'elevata efficienza nell'esecuzione dei programmi**:
    Il VMM deve permettere l'esecuzione diretta delle istruzioni impartite dalle macchine virtuali, quindi
    le istruzioni non privilegiate vengono eseguite direttamente in hardware senza coinvolgere il VMM
3. **Garantire la stabilità e la sicurezza dell'intero sistema**:
    Il VMM deve sempre rimanere sempre nel pieno controllo delle risorse hardware, e i programmi in 
    esecuzione nelle macchine virtuali non possono accedere all'hardware in modo privilegiato

#### Parametri e classificazione
- **Livello** dove è collocato il VMM:
    - **VMM di sistema**: eseguono direttamente sopra l'hardware dell'elaboratore (vmware esx, xen, kvm)
    - **VMM ospitati**: eseguiti come applicazioni sopra un S.O. esistente (parallels, virtualbox)
- **Modalità di dialogo**: per l'accesso alle risorse fisiche tra la macchina virtuale ed il VMM:
    - **Virtualizzazione pura** (vmware): le macchine virtuali usano la stessa interfaccia 
    dell'architettura fisica
    - **Paravirtualizzazione** (xen): il VMM presenta un'interfacca diversa da quella dell'architettura HW


..
guest e host
..
..
..

#### Ring di protezione
La CPU prevede due livelli di protezione: **supervisore o kernel (0)** e **utente (>0)**.

Ogni ring corrisponde a una diversa modalità di funzionamento del processore:
    - a livello 0 vengono eseguite le istruzioni privilegiate della CPU
    - nei ring di livello superiore a 0 le istruzioni privilegiate non vengono eseguite

Alcuni progrmmi sono progettati per eseguire nel ring 0, ad esempio il Kernel del S.O. infatti
è l'unico componente che ha pieno controllo dell'hardware.

##### VMM (vmm di sistema)
In un sistema virtualizzato il VMM deve essere l'unica componente in grado di mantenere il controllo
completo dell'hardware. Infatti solo il VMM opera nello stato supervisore, mentre il S.O. e le 
applicazioni eseguono in un ring di livello superiore.

Sorgono però due problemi:
- **Ring deprivileging**: il s.o. della macchina virtuale esegue in un ring che non gli è proprio
- **Ring compression**: se i ring utilizzati sono solo 2, applicazioni e s.o. della macchina virtuale
eseguono allo stesso livello: scarsa protezione tra spazio del s.o. e delle applicazioni.

##### Ring Deprivileging
