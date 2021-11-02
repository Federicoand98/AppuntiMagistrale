

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


..
..
..
..

# 5/10/2021

## Programmazione Concorrente
La programmazione concorrente è l'insieme delle tecniche, metodologie e strumenti per il supporto all'esecuzione di sistemi software composti da insiemei di attività svolte
simultaneamente.

hardware

### Tipi di Applicazioni
- **Multithreaded**: applicazioni strutturate come un insieme di processi con lo scopo di aumentare l'efficienza, ridurre la complessità e la programmazione. I processi posssono
condividere variabili, generalmente esistono più processi che processori; e i processi vengono schedulati ed eseguiti indipendentemente.
- **Sistemi multitasking/Sistemi Distribuiti**: le componenti dell'applicazione (task) vengono eseguite sui nodi, generalemente virtuali, collegati tramite opportuni mezzi
di interconnessione. E i processi comunicano scambiandosi messaggi.
- **Applicazioni Parallele**: applicazioni che sfruttano al massimo il parallelismo disponibile a livello HW. Infatti vengono eseguite su sistemi specificatamente paralleli
come HPC o processori vettoriali, facendo largo uso di algoritmi paralleli. A seconda del modello architetturale, l'esecuzione è portata avanti da istruzioni/thread/processi paralleli
che interagiscono utilizzando specifiche librerie.

### Processi non sequenziali e tipi di interazione
- **Algoritmo**: procedimento logiche che deve essere eseguito per risolvere in determinato problema
- **Programma**: descrizione di un algoritmo mediante un opportuno formalismo (linguaggio di programmazione), che rende possibile l'esecuzione dell'algoritmo da parte di un
particolare elaboratore
- **Processo**: insieme **ordinato degli eventi** cui dà luogo un elaboratore quando opera sotto il controllo di un programma.
- **elaboratore**: entità astratta realizzata in HW e parzialmente in SW in grado di eseguire programmi
- **Evento**: esecuzione di un'operazione tra quelle apparteneti all'insieme che l'elaboratore sa riconoscere ed eseguire, ogni evento determina una **transizione di stato**
dell' elaboratore.

Quindi un programma non descrive un processo, ma un insieme di rpocessi, ognuno dei quali è relativo all'esecuzione del programma da parte dell'elaboratore per un determinato....

**Processo Sequenziale**: sequenza di stati attraverso i quali passa l'elaboratore durante l'esecuzione di un programma.

Un processo può essere rappresentato tramite un grafo orienteto detto **gtafo di precedenza del processo**, costituito da nodi ed archi orientati:
- i nodi del grafo rappresentano i singoli eventi del processo
- gli archi orientati identificano le precedenze temporali tra tali eventi
Ogni nodo rappresenta quindi un evento corrispondente all'esecuzione di un'operazione tra quelle appartenti all'insieme che l'eleaboratore sa riconoscere ed eseguire

**Processi non sequenziali**:
L'insieme degli eventi che lo descrive è ordinato secondo una relazione d'ordine parziale. Quindi per descrivere un processo non sequenziale utilizziamo
un **grado di precedenza ad ordinamento parziale**.

# 6/10/21
Per poter eseguire un processo non sequenziale, ovviamente, richiede un elaboratore non sequenziale.

**Processi Interagenti**: le interazioni tra processi di lettura, elaborazione e scruttira sono dettati da uno scambio di informazioni.

**Tipi di Decomposizione**
La decomposizione di uno sgrafo di precedenza ad ordinamento parziale può essere effettuato in modi differenti, orizzontalmente (quindi per archi), oppure
verticalmente (per processi), generalmente si sceglie quello che permette di avere il minimo numero di interazioni possibili.

#### Interazione tra processi
Comprende tutte le interazoini prevedibili e desderate, insite quindi nella logica degli algoritmi. Generalmente
prevede scambio di informazioni, o dati (quindi messaggi, e quindi comunicazione), oppure segnali temporali (senza trasfermento di dati).

**Competizione**:
La macchina concorrente su cui i processi sono eseguiti mette a disposizione un numero linitato di risorse condivise tra i processi.
L'obiettivo della competizione è il coordinamente dei processi nell'accesso alle risorse condivise. Quindi, nel caso determinate risorse non
possono essere accedibili a più processi contemporaneamente, è necessario quindi prevedere dei meccanismi che facciano rispettare le regole di accesso
delle risorse e quindi regolino la competizione (in breve è una forma di sincronizzazione in più).

Un esempio è la Mutua Esclusione, nel quale, ad esempio, due processi devono utilizzate in determinati istanti una risorsa condivisa, ma questa
risorsa può essere acceduta solamente da un processo alla volta. Quindi gli accessi alla risorsa devono essere fatti in modo mutuamente esclusivo, ovvero
un processo alla volta.

**Sezione Critica**:
Sequenza di istruzioni con le quali un processo accede a un oggetto condiviso con altri oggetti. Ad un oggetto può essere associata ad una sola
sezione critica (usata da tutti i processi) o più sezioni critiche (classe di sezioni critiche). **Sezioni critiche appartenenti alla stessa classe**
**devono escludersi mutuamente nel tempo**.

L'interazione tra processi può essere di puù tipi:
- Cooperazione: sincronizzazione diretta o esplicita
- Competizione: sincronizzazione indiretta o implicita
- Interferenza: interazione provocata da errori di programmazione (deadlock), interazione non prevista e non desiderata.

### Architetture e Linguaggi per la programmazione concorrente

Proprietà che un linguaggio per la programmazione concorrente deve avere:
- contenere appositi costrutti con i quali è possibile dichiarare moduli di programma destinati ad essere eseguiti come processi sequenziali distinti
- non tutti i processi vengono eseguiti contemporaneamente. Acluni processi vengono svolti solamente se si verificano determinate condizioni
- cose

Una macchina concorrente è costituita da HW e componenti SW che permettono la multiprogrammazione. Ovviamente, in una Macchina astratta M, oltre
ai processi di multiprogrammazione e sincronizzazioen è presente anche il meccanismo di **protezione**, e quindi il meccaniscmo di controllo
degli accessi alle risorse, in grado di:
- rilevare eventuali interfrerenze tra i processi
- pu essere realizzato in HW o in SW nel supporto a tempo di esecuzione
- capabilities e liste di controllo degli accessi

Il kernel offre supproto a tempo di esecuzione di un linguaggio concorrente.
Nel kernel sono sempre presenti due funzionalità base:
- meccanismo di multiptogrammazione
- meccanismo di sincronizzazione e comunicazione

**Architettura di M**:
Due diverse organizzazioni logiche:
- gli elaboratori di M sono collegati ad un'unica memoria principale (sistemi multiprocessore)
- gli elaboratori di M sono collegati da una sottorete di comunicazione, senza memoria comune

Queste due organizzazioni logiche di M definiscono due modelli di interazione tra i processi:
- Modello a memoria comune, nel quale l'interazione tra i processi avviene tramite oggetti contenuti nella memoria comune (modello ad ambiente globale)
- Modello a scambio di messaggi, nel quale la comunicazione e la sincronizzazione tra i processi si basa sullo scambio di messaggi sulla rete che collega
i vari elaboratori.

### Costrutti linguistici per la specifica della concorrenza

#### Fork/Join
L'esecuzione di una **fork** (non la system call unix) coincide con la creazioen e l'attivazione di un processo che inizia la propria esecuzioen in parallelo
con quella del processo chiamante.
La **join** è una primitiva di sincronizzazione che permette di determinare quando un processo, creato tramite la fork, ha terminato il suo compito,
sincronizzandosi con tale evento.

#### Cobegin-Coend
Alternativa al modello fork/join, prende ispirazione al modello di programmazione strutturata.

s_0;
**cobegin**
    s_1;
    s_2;
    s_2;
**coend**
s_4;

Le istruzioni S1, S2,..Sn sono eseguite in parallelo. Ogni si può contenere altre istruzioni cobegin..coend al suo interno.

### Proprietà dei programmi
**Traccia dell'esecuzione**: sequenza degli stati attraversati dal sistema di elaborazione durante l'esecuzione del programma
**Stato**: insieme dei valori delle variabili definite nel programma più variabili implicite (es valore del Program Counter).

**Proprietà safety**: proprietà che garantisce cje durante l'esecuzioen di P non si entrerà mai in uno stato errato
**Propietà liveness**: 
