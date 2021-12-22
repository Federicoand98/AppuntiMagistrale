package main

import (
	"fmt"
	"math/rand"
	"time"
)

const MAXBUFF = 100
const MAX_P = 100
const MAX_V = 150
const Y = 5 // acquisti alla volta
const Z = 4 // consegna vuoti alla volta
const X = 3 // consegna acqua alla volta
const K = 30
const BANCOMAT = 0
const BONIFICO = 0
const CONTANTI = 1
const PA = 2
const PV = 1

type Richiesta struct {
	id  int
	ack chan int
}

//definizione canali
var done = make(chan bool)
var termina = make(chan bool)
var acquirente_contanti = make(chan Richiesta, MAXBUFF)
var acquirente_bancomat = make(chan Richiesta, MAXBUFF)
var fornitore_contanti = make(chan Richiesta, MAXBUFF)
var fornitore_bonifico = make(chan Richiesta, MAXBUFF)

func when(b bool, c chan Richiesta) chan Richiesta {
	if !b {
		return nil
	}
	return c
}

func acquirente(id int) {
	fmt.Printf("[Acquirente %d]: avviato.", id)

	richiesta := Richiesta{id, make(chan int)}
	pagamento := rand.Intn(2)

	switch pagamento {
	case CONTANTI:
		acquirente_contanti <- richiesta
		fmt.Printf("[Acquirente %d]: acquistate %d bottiglie in CONTANTI", id, Y)
	case BANCOMAT:
		acquirente_bancomat <- richiesta
		fmt.Printf("[Acquirente %d]: acquistate %d bottiglie in BANCOMAT", id, Y)
	default:
		fmt.Printf("[Acquirente %d]: error", id)
		done <- true
		return
	}

	time.Sleep(time.Duration(rand.Intn(2)+1) * time.Second)

	spesa := <-richiesta.ack

	if spesa >= 0 {
		fmt.Printf("[Acquirente %d]: ho speso %d e consegnato %d bottiglie vuote", id, spesa, Z)
	} else {
		fmt.Printf("[Acquirente %d]: errore...", id)
	}

	done <- true
	return
}

func fornitore(id int) {
	fmt.Printf("[Fornitore %d]: avvitao.", id)

	for {
		rifornimento := Richiesta{id, make(chan int)}
		pagamento := rand.Intn(2)

		switch pagamento {
		case CONTANTI:
			fornitore_contanti <- rifornimento
			fmt.Printf("[Fornitore %d]: rifornimento di %d bottiglie, pagamento in CONTANTI", id, X)
		case BONIFICO:
			fornitore_bonifico <- rifornimento
			fmt.Printf("[Fornitore %d]: rifornimento di %d bottiglie, pagamento in BONIFICO", id, X)
		default:
			fmt.Printf("[Fornitore %d]: error", id)
			done <- true
			return
		}

		incasso := <-rifornimento.ack
		if incasso == -1 {
			fmt.Printf("[Fornitore %d] termina...", id)
			done <- true
			return
		}

		fmt.Printf("[Fornitore %d]: incassato %d", id, incasso)
		time.Sleep(time.Duration(rand.Intn(5)+1) * time.Second)
	}
}

func ditta() {
	fmt.Printf("[Ditta]: start")

	running := true
	cassa := X * PV
	contoCorrente := X * PV
	numPiene := 0
	numVuote := 0

	for {
		select {
		// acquirente
		case x := <-when((running && numPiene >= Y && numVuote+Z <= MAX_V && (cassa < K || len(acquirente_bancomat) == 0)), acquirente_contanti):
			spesa := Y * PA
			cassa = cassa + spesa
			numPiene = numPiene - Y
			numVuote = numVuote + Z
			fmt.Printf("[Ditta]: un aquirente ha pagato in contanti. NumPiene: %d, NumVuote: %d, Cassa: %d, ContoCorrente: %d", numPiene, numVuote, cassa, contoCorrente)
			x.ack <- spesa

		case x := <-when((running && numPiene >= Y && numVuote+Z <= MAX_V && (cassa >= K || len(acquirente_contanti) == 0)), acquirente_bancomat):
			spesa := Y * PA
			contoCorrente = contoCorrente + spesa
			numPiene = numPiene - Y
			numVuote = numVuote + Z
			fmt.Printf("[Ditta]: un aquirente ha pagato in bancomat. NumPiene: %d, NumVuote: %d, Cassa: %d, ContoCorrente: %d", numPiene, numVuote, cassa, contoCorrente)
			x.ack <- spesa

		// fornitore
		case x := <-when((running && numPiene+Y <= MAX_P && numVuote >= Z && (cassa >= K || len(fornitore_bonifico) == 0)), fornitore_contanti):
			numVuote = 0
			numPiene = numPiene + Y
			incasso := X * PV
			cassa = cassa - incasso
			fmt.Printf("[Ditta]: un fornitore ha pagato in contanti. NumPiene: %d, NumVuote: %d, Cassa: %d, ContoCorrente: %d", numPiene, numVuote, cassa, contoCorrente)
			x.ack <- incasso

		case x := <-when((running && numPiene+Y <= MAX_P && numVuote >= Z && (cassa < K || len(fornitore_contanti) == 0)), fornitore_bonifico):
			numVuote = 0
			numPiene = numPiene + Y
			incasso := X * PV
			contoCorrente = contoCorrente - incasso
			fmt.Printf("[Ditta]: un fornitore ha pagato in bonifico. NumPiene: %d, NumVuote: %d, Cassa: %d, ContoCorrente: %d", numPiene, numVuote, cassa, contoCorrente)
			x.ack <- incasso

		// terminazione
		case x := <-when(!running, acquirente_contanti):
			x.ack <- -1
		case x := <-when(!running, acquirente_bancomat):
			x.ack <- -1
		case x := <-when(!running, fornitore_contanti):
			x.ack <- -1
		case x := <-when(!running, fornitore_bonifico):
			x.ack <- -1

		case <-termina:
			if running {
				fmt.Println("[Ditta]: start termination")
				running = false
			} else {
				fmt.Println("[Ditta]: terminated..")
				done <- true
				return
			}
		}
	}
}

func main() {
	var numAcquirenti int
	var numFornitori int
	rand.Seed(time.Now().UTC().UnixNano())

	fmt.Printf("\n[main] Quanti acquirenti? \n")
	fmt.Scanf("%d", &numAcquirenti)

	fmt.Printf("\n[main] Quanti fornitori? \n")
	fmt.Scanf("%d", &numFornitori)

	done = make(chan bool, numAcquirenti+numFornitori)

	go ditta()

	for i := 0; i < numAcquirenti; i++ {
		go acquirente(i)
	}

	for i := 0; i < numFornitori; i++ {
		go fornitore(i)
	}

	for i := 0; i < numAcquirenti; i++ {
		<-done
	}

	termina <- true

	for i := 0; i < numFornitori; i++ {
		<-done
	}

	termina <- true
	<-done
	fmt.Println("[main]: terminated")
}
