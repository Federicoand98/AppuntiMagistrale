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

**Virtualizzazione della CPU**
Il VMM definisce un'arcihtettura virtuale simile a quella del processore, nella quale però le istruzioni privilegiate vengono sostituite dalle
hypercall. Nel quale l'invocazione di una hypercall determina il passaggio da guest a xen (ring 1 -> ring 0).
Il VMM si occupa dello scheduling delle macchine virtuali: **Borrowed Virtual Time scheduling algorithm**, algoritmo genral purpose che consente di 
scedulazioni efficienti.

**Virtualizzazione dell'I/O**
immagine

- **back-end driver**: per ogni dispositivo, il suo driver è isolato all'interno di un ap articolare macchina virtuale, tipicamente Dom0. E ha accesso
    diretto all'HW.
- **front-end driver**: ogni guest rpevede un driver virtuale semplificato che consente l'accesso al devie tramite il back-end.

I punti a favore di questa soluzione sono la portabilità, isolamente e semplificazione del VMM. Mentre i punti a sfavore sono: necessità di comunicazione
con il back-end (asynchronous I/O rings).

**Gestione di interruzioni ed eccezioni**
La gestione delle interruzioni viene viertualizzata in questo modo: il vettore delle interruzioni punta direttamente alle routine del kernel guest: ogni
interruzione viene gestita direttamente dal guest.
Un caso particolare riguarda il page-fault:
- la gestione non può essere delegata completamente al guest, perchè richiede l'accesso al registro **CR2**, contenete l'indirizzo che ha provocato
    il page fault
- aaaaaa

Quidni per gestire il page-fault, 'handler punta a codice xen (execuzione nel ring 0). La routine di gestione eseguita da xen legge in contenuto di CR2
e lo copia aaaaaa..... slide

**Live migration in Xen**
La migrazione in xen è una migrazione **guest-based**, nel quale il comando di migrazione viene eseguito da un demone di migrazione nel domain0 del
server di origine della macchina da migrare. La realizzazione è basata su pre-copy, inoltre le pagine da migrare vengono compresse per ridurre
l'occupazione di banda.

**fine capitolo virtualizzazione**

# La Protezione nei Sistemi Operativi

**Sicurezza**: riguarda l'insieme delle tecniche per regolamentare l'accesso degli utenti al sistema di elaborazione. La sicurezza impedisce accessi
non autorizzati al sistema e i conseguenti tentativi dolosi di alterazione e distruzione di dati.

**Protezione**: insieme di attività volte a garantire il controllo dell'accesso alle risorse logiche e fisiche da parte degli utenti autorizzati
all'uso di un sistema di calcolo.

La sicurezza mette a disposizione meccanismi di **identificazione, autenticazione, ...**

Per rendere un sistema sicuro è necessario stabilire per ogni utente autorizzato:
- quali siano le risore alle quali può accedere
- con quali operazioni può accedervi
Tutto ciò è stabilito dal sistema di protezione attraverso delle tecniche di controllo dell'accesso.

In un sistema il controllo degli accessi si esprime tramite la definizione di tre livelli concettuali:
- modelli
- politiche
- meccanismi

#### Modelli
Un modello di protezione definisce i soggetti, gli oggetti e i diritti d'accesso:
- **oggetti**: costituiscono la parte passiva, cioè le risorse fisiche e logiche alle quali si può accedere e su cui si può operare.
- **soggetti**: rappresentano la parte attiva di un sistema, cioè le eintità che possono richiedere l'accesso alle risorse (utenti e processi)
- **diritti d'accesso**: sono le operazioni con le quali è possibile operare sugli oggetti

(Un soggetto può avere diritti d'accesso sia per gli oggetti che per gli altri soggetti)

Ad ogni soggetto è associato un **dominio di protezione**, che rappresenta l'ambiente di protezione nel quale il soggetto esegue. Quindi il dominio
indica i diritti d'accesso posseduti dal sogetto nei confronti di ogni risorsa.
Un dominio di protezioen è unico per ogni soggetto, mentre un processo può eventualmente cambiare dominio durante la sua esecuzione.

#### Politiche
Le **politiche di protezione** definiscono le regole con le quali i soggetti possono accedere agli oggetti
Classificazione delle politiche:
- **discretional access control (DAC)**: il creatore di un oggetto controlla i diritti di accesso per quell'oggetto (unix). La definizione delle politiche è
    decentralizzata.
- **mandatory access control (MAC)**: i diritti di accesso vengono definiti in modo centralizzato. Ad esempio in installazioni di alta sicurezza
- **role based access control (RBAC)**: ad un ruolo sono assegnati specifici diritti di accesso sulle risorse. Sli utenti possono appartenere a diversi
    ruoli. I diritti attribuiti ad ogni ruolo vengono assegnati in modo centralizzato

**Principio del privilegio minimo**: ad ogni soggetto sono garantiti i diritti d'accesso solo agli oggetti strettamente necessari per la sua esecuzione
(POLA: principle of least authority). il POLA è una caratteristicha desiderabile in ogni sistema di controllo.

#### Meccanismi
I **meccanismi di protezione** sono gli strumenti necessari a mettere in atto una determinata politica.
Principi di realizzazione:
- **Flessibilità del sistema di protezione**: i meccanismi devono essere sufficientemente generali per consentire l'applicazione di diverse politiche 
    di protezione
- **Separazione tra meccanismi e politiche**: la politicha definische "cosa va fatto" ed il meccanismo "come va fatto". Ovviamente è desiderata la
    massima indipendenza tra le due componenti.

#### Dominio di protezione
Un dominio definisce un insiem edi coppie, ognuna contenente l'identificatore di un oggetto e l'insieme delle operazioni che il soggetto associato al
dominio può eseguire su ciascun oggetto

$$ D(S) = {<o, diritti> | o è un oggetto, diritti è un insieme di operazioni} $$


#### Modello di Grahmm-Denning
Questo modello forsnisce una serie di comandi che garantiscono la modifica controllata dello stato di protezione:
- create object
- delete object
- create subject
- delete subject
- read access right
- grant access right
- delete access right
- tranfer access right

##### Diritti
**Diritto Owner**:
Il diritto owner permette l'assegnazioen di qualunque diritto di accesso su un oggetto X ad un qualunque soggetto Sj da parte di un soggetto Si. L'operazione è consentita solo
se il diritto owner appartiene a A[Si, X]

**Diritto Control**:
Eliminazione di un diritto di accesso per un oggetto X nel dominio di Sj da parte di Si. L'operazione è consentita solo se il diritto control appartiene a A[Si, Sj], oppure owner
appartiene a A[Si, X].

**Cambio di dominio: switch**
Il cambio di dominio permette che un processo che esegue nel dominio del soggetto si può commutare al dominio di un altro soggetto Sj.
L'operazione è consentita solo se il diritto switch appartiene a A[Si, Sj].

#### Realizzazione della matrice delgi accessi
La matrice degli accessi è una notazione astratta che rappresenta lo stato di protezione. Nella rappresentazione concreta è necessario considerare: la dimensione della matrice e matrice 
sparsa.

La rappresentazione concreta della matrice degli accessi deve essere ottimizzata sia riguardo all'occupazione di memoria sia rispetto all'efficienza nell'accesso e nella gestione della
informazioni di protezione.
Ci sono principalmente di approcci:
- **Access Control List (ACL)**: rappresentazione per colonne, per ogni oggetto è associata una lista che contiene tutti i soggetti che possono accedere all'oggetto, con i relativi diritti
    d'accesso per l'oggetto
- **Capability List**: rappresentazione per righe, ad ognin soggetto è associata una lista che contiene gli oggetti accessibili dal soggetto ed i relativi diritti d'accesso.

##### Access Control List
La lista degli accessi per ogni oggetto è rappresentata dall'insieme delle coppie: **<soggetto, insieme dei diritti>**
limitatamente ai soggetti con un insieme non vuoto di diritti per l'oggetto.
Quando deve essere eseguita un'operazione M su un oggetto Oj, da parte di Si, si cerca nella lista degli accessi **<Si, Rk>, con M appartenente a Rk**
La ricerca può essere fatta preventivamente in una lista di default contenete i diritti di accesso applicabili a tutti gli oggetti. 
Se in entrambi i casi la risposta è negativa, l'accesso è negato.

**Utenti e Gruppi**
Generalmente ogni soggetto rappresenta un singolo utente. Molti sistemi hanno il concetto di **gruppo di utenti**. I gruppi hanno un nome e possono essere inclusi nella ACL.
In questo caso l'entry in ACL ha la forma:
**UID_1, GID_1 : <insieme di diritti>**
**UID_2, GID_2 : <insieme di diritti>**
Dove UID è lo user identifier e GID è il group identifier.

In certi casi il gruppo identifica un ruolo: uno stesso utente può appartenere a gruppi diversi e quindi  con diritti diversi. In questo caso, quando un utente accede, specifica il
gruppo di appartenenza.

##### Capability List
La lista delle capability, per ogni soggetto, è la lista di elementi ognuno dei quali:
- è associato a un oggetto a cui il soggetto può accedere
- contiene i diritti di accessi consentiti su tale oggetto

Ogni elemento della lista prende il nome di **capability**. Il quale di compone di un identificatore (o un indirizzo) che indica l'oggetto e la rappresentazione dei vari diritti d'accesso.
Quando S intende eseguire un'operazione M su un oggetto Oj: il meccanismo di protezione controlla se nella lisra delle capability associata a S ne esiste una relativa ad Oj che abbia
tra i suoi diritti M.

Ovviamente le Capability List devono essere protette da manomissioni, ed è possibile in diversi modi:
- la capability list viene gestita solamente da s.o.; l'utente fa riferimento ad un puntatore (capability) che identifica la sua posizione nella lista appartenete allo spazio del kerner
- Architettura etichettata: a livello HW, ogni singola parola ha bit extra, che esprimono la protezione su quella cella di memoria. Ad esempio, se è una capability, deve essere
protetta da scritture non autorizzate.
I bit tag non sono utilizzari dall'aritmetica, dai confronti e da altre istruzioni normali e può essere modificato solo da programmi che agiscono in modo kernel.

##### Revoca dei diritti di accesso
In un sistema di protezione dinamica può essere necessario revocare i diritti d'accesso per un oggetto. La revoca può essere di tre tipi:
- **generale o selettiva**: cioè valere per tutti gli utenti che hanno quel diritto di accesso o solo per un gruppo
- **parziale o totale**: cipè riguardare un sottoinsieme di diritti per l'oggetto, o tutti
- **temporanea o permanente**: cioè il diritto di accesso non sarà più disponibile, oppure potrà essere successivamente riottenuto

**Revoca del diritto per un oggetto con ACL**:
Si fa riferimento alla ACL associata all'oggetto e si cancellano i diritti di accesso che si vogliono revocare

**Revoca del diritto per un oggetto con Capability List**:
Più complicato rispetto ad ACL. È necessario verificare per ogni dominio se contiene la capability con riferimento all'oggetto considerato.