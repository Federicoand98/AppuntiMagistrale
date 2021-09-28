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

# 22/09/2021
##### Ring Deprivileging
Con Ring Deprivilenging si indica una situazione nel quale l'esecuzione di istruzioni privilegiate richieste dal sistema operativo nell'ambiente guest non
possono essere eseguite in quanto richiederebbero un ring 0, ma il kernel della macchina virtuale esegue in un ring di livello superiore (foto telefono 1)

Una possibile prima soluzione è il **Trap & Emulate**: nel quale se il guest tenta di eseguire un'istruzione privilegiata
- la CPU notifica un'eccezione al VMM (**trap**) e gli trasferisce il controllo
- il VMM controlla la correttezza dell'operazione richiesta e ne emula il comportamento (**emulate**)

Quindi in poche parole la CPU notifica e delega al VMM il controllo e l'esecuzione dell'istruzione privilegiata.

Esempio:
Il guest tenta di disabilitare le interruzioni (popf), se la richiesta della macchina virtuale fosse eseguita direttamente sulla CPU sarebbero disabilitati
tutti gli interrupt di sistema e quindi il VMM non potrebbe riottenere il controllo. Invece, con Trap&Emulate riceve la notifica di tale richiesta e ne emula
il comportamento sospendendo gli interrupt solamente per la macchina virtuale richiedente.

#### Supporto HW alla virtualizzazione
L'archietettura della CPU si dice **naturalmente virtualizzabile** se e solo se prevede l'invio di trap al VMM per ogni istruzione privilegiata invocata da un
livello di protezione differente dal quello del VMM.

Se la CPU è naturalmente virtualizzabile viene implementato il trap&emulate, altrimenti, se non è virtualizzabile vi sono 2 possibilità: **Fast Binary Translation** e 
**Paravirtualizzazione**.

##### Fast Binary Translation
Il VMM scansiona dinamicamente il codice dei sistemi operativi guest prima dell'esecuzione per sostituire a run time i blocchi contenenti istuzioni privilegiate
in blocchi equivalenti dal punto di vista funzionale e contenenti chiamate al VMM. Inoltre i blocchi tradotti sono eseguiti e conservati in cache per eventuali
riusi futuri. (SISTEMARE)

(immagine slide 33)

Il principale limite della Fast Binary Translation è che la traduzione dinamica è molto costosa. Però, con questa tecnica, ogni macchina virtuale è una esatta
copia della macchina fisica, con la possiblità di installare gli stessi s.o. di architetture non virtualizzate.

##### Paravirtualizzazione
Il VMM (hypervison) offre al sistema operatico gurst un'interfaccia virtuale (ovviamente differente da quello hardware del processore) chiamata **hypercall API**
alla quale i s.o. guest devono rifersi per avere accesso alle risorse (system call).
Queste Hypercall API permettono di:
- richiedere l'esecuzione di istruzioni privilegiate, senza generare un interrupt al VMM
- i kernel dei s.o. guest devono quindi essere modificati per avere accesso all'interfaccia del particolare VMM
- l astruttura del VMM è semplificata perchè non deve più preoccuparsi di tradurre dinamicamente i tentativi di operazioni privilegiate dei s.o. guest

Le prestazioni rispetto alla Fast Binary Translation sono notevolmente superiori, però ovviamente c'è una necessità di porting dei dei s.o. guest (non sempre facile).

(aggiungere protezione processore)

#### Gestione di Macchine Virtuali
Il compito principale del VMM è la gestione delle macchine virtuali, e quindi **creazione**, **spegnimento/accensione**, **eliminazione**, e **migrazione live**.

Una Macchina Virtuale può trovarsi in differenti stati:
- **Running** (o attiva): la macchina è accesa e occupa memoria nella ram del server sul quale è allocata
- **Inactiva** (powered off): la macchina virtuale è spenta ed è rappresentata da un file immagine nel filesystem del server
- **Paused**: la macchina virtuale è in attesa di un evento
- **Suspended**: la macchina virtuale è stata sospesa dal VMM, il suo stato e le risorse utilizzate sono all'interno di un file immagine nel filesystem del server.
    L'uscita dallo stato suspended avviene tramite l'operazione di **resume** da parte del VMM.

(immagine stati slide 40)


# 28/09/21

migrazione live

#### Suspend/Resume
Il VMM può mettere in **suspend** una VM salvandone lo stato in memoria secondaria. Invece con l'operazione di **resume** la VM esce dallo stato di 
suspend e il VMM recupera lo stato dalla memoria secondaria. Resume e Suspen permettono e facilitano la **Migrazione**, ovvero il trasferimento
della macchina da un nodo ad un'altro.

#### Precopy
La migrazione viene effettuatat in 6 fasi:
- **pre-migrazione**: individuazione della VM da migrare e dell'host (B) di destinazione
- **reservation**: viene inizializzata una VM (container) sul server di destinazione
- **pre-copia iterativa delle pagine**: viene eseguita una copia nell'host B di tutte le pagine allocate in memoria sull'host A per la VM da migrare, 
    successivamente vengono iterativamente copiate da A a B tutte le pagine modificate (dirty pages) fino a quando il numero di dirty pages diventa
    inferiore ad un soglia prestabilita
- **sospensione della VM e copia dello stato + dirty pages** da A a B
- **commit**: la VM viene eliminata dal server A
- **resume**: la VM viene attivata nel server B

In alternativa alla pre-copy una soluzioen può essere la **post-copy**, nel quale la VM viene sospesa, copiata interamente non iterativamente e poi 
eliminata. La principale differenza tra pre-copy e post-copy è che con il meccanismo della pre-copy viene ridotto al minimo il **downtime** della VM.

##### Piccolo approfondimento su XEN
Xen è un VMM open source che sfrutta i principi di paravirtualizzazione. È nato come progetto accademico nell'Università di Cambridge, successivamente
viene effettuato un porting di Linux su Xen. È stato modificato il kernel linux per poter interagire con le API di Xen.

L'architettura di Xen si basa sulla virtualizzaizione di sistema, nel quale è presente un hypervisor posizionato direttamente al di sopra dell'HW.
Le VM di xen vengono chiamate **domain**, è sempre presente un **domain 0**, ovvero quella VM utilizzata per lo operazioni privilegiate, quindi
quella macchina speciale che permette di controllare e operare sull'intero sistema.

Il VMM o hypervisor quindi si occupa di virtualizzare la CPU, memoria e dispositivi di I/O. Xen mette a disposizione un'interfaccia per poter usufruire
del VMM.

- Paravirtualizzazioen di Xen:
    - le VM eseguono direttakmente le istruzioni non privilegiate
    - l'esecuzione di istruzioni privilegiate viene delegata al VMM tramite chiamate al VMM
- protezione (x86):
    - i s.o guest sono collocati nel ring 1
    - VMM collocata nel ring 0

**Xen: Gestione della memoria e paginazine**
- gestione della memoria:
    - i s.o. gestiscono la memoria virtuale mediante i tradizionali meccanismi di paginazione
    - x86: page faults gestiti direttamente a livello HW (TLB)
- soluzione adottata: tabelle delle pagine delle VM:
    - vengono mappate nella memoria fisica dal VMM (shadow page tables)
    - non possono essere accedute in scrittura dai kernel guest, ma solo dal VMM
    - sono accessibili in modalità read-only anche dai guest
    In caso di necessità di update interviene direttamente il VMM che valida re richieste di update dei guest e le esegue
- memory split:
    - Xen risiede nei primi 64 MD del virtual address space
    - in questo modo non è necessario eseguire un TLB flush per ogni hypercall

I guest OS si occupano della paginazione, delegano al VMM la scrittura delle page table entries, inoltre le tabelle vengono create e verificate dal
VMM su richiesta dei guests, una volta create quindi queste tabelle rimangono read-only.

**Creazione di un processo** (fork)
Il s.o. guest richiede ina nuova tabella delle pagine al VMM:
    - alla tabella vengono aggiunte le pagine appartenenti al segmento di xen
    - xen registra la tabella delle pagine e acquisisce il diritto di scrittura esclusiva
    - ogni successiva update dal parte del guest provocherà un **protection-fault**, la cui gestion comporterà la verifica e l'aggiornamento della table.

In quanto la paginazione è a carico dei guest, occorre un meccanismo efficiente che consenta al VMM di reclamare ed ottenere dalle altre VM pagine di
memoria meno utilizzare.
Una soluzione può essere che su ogni VM è in esecuzione un processo (**baloon process**) che comunica direttamente con il VMM, e che viene interpellato
ogni volta che il VMM ha bisogno di ottenere nuove pagine ad esempio l'archiviazione di una VM.