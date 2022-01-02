package main

import (
	"fmt"
	"math/rand"
	"time"
)

/////////////////////////////////////////////////////////////////////
// Costanti
/////////////////////////////////////////////////////////////////////
const MAXBUFF = 100
const MAXPROC = 10
const MAX = 5 // capacità
const CONTANTI = 0
const BONIFICO = 1
const BANCOMAT = 1
const PAP = 10
const PVP = 8
const N = 150
const X = 50

/////////////////////////////////////////////////////////////////////
// Strutture Dati
/////////////////////////////////////////////////////////////////////
type Richiesta struct {
	id  int
	ack chan int
}

/////////////////////////////////////////////////////////////////////
// Canali
/////////////////////////////////////////////////////////////////////
var fornitore_contanti = make(chan Richiesta, MAXBUFF)
var fornitore_bonifico = make(chan Richiesta, MAXBUFF)
var acquirente_contanti = make(chan Richiesta, MAXBUFF)
var acquirente_bancomat = make(chan Richiesta, MAXBUFF)

/////////////////////////////////////////////////////////////////////
// Join Goroutine
/////////////////////////////////////////////////////////////////////
var done chan bool
var termina = make(chan bool)

/////////////////////////////////////////////////////////////////////
// Funzioni Ausiliarie
/////////////////////////////////////////////////////////////////////
// se si usa struttura dati modificare la when
func when(b bool, c chan Richiesta) chan Richiesta {
	if !b {
		return nil
	}
	return c
}

func whenInt(b bool, c chan int) chan int {
	if !b {
		return nil
	}
	return c
}

func sleepRandTime(timeLimit int) {
	if timeLimit > 0 {
		time.Sleep(time.Duration(rand.Intn(timeLimit)+1) * time.Second)
	}
}

/////////////////////////////////////////////////////////////////////
// Goroutine
/////////////////////////////////////////////////////////////////////
func agricoltore(id int) {
	fmt.Printf("[Agricoltore %d]: avviato\n", id)

	for {
		richiesta := Richiesta{id, make(chan int)}

		pagamento := rand.Intn(2)

		switch pagamento {
		case CONTANTI:
			fornitore_contanti <- richiesta
			fmt.Printf("[Agricoltore %d]: rifornimento di 10kg di patate, pagamento in contanti\n", id)
		case BONIFICO:
			fornitore_bonifico <- richiesta
			fmt.Printf("[Agricoltore %d]: rifornimento di 10kg di patate, pagamento con bonifico\n", id)
		default:
			fmt.Printf("[Agricoltore %d]: errore\n", id)
			done <- true
			return
		}

		incasso := <-richiesta.ack

		if incasso == -1 {
			fmt.Printf("[Agricoltore %d]: termina\n", id)
			done <- true
			return
		}

		fmt.Printf("[Agricoltore %d]: incassato %d\n", id, incasso)
		sleepRandTime(7)
	}
}

func cliente(id int) {
	fmt.Printf("[Cliente %d]: avviato\n", id)

	richiesta := Richiesta{id, make(chan int)}
	pagamento := rand.Intn(2)

	switch pagamento {
	case CONTANTI:
		acquirente_contanti <- richiesta
		fmt.Printf("[Cliente %d]: acquistato 1kg di patate in contanti\n", id)
	case BANCOMAT:
		acquirente_bancomat <- richiesta
		fmt.Printf("[Cliente %d]: acquistato 1kg di patate in bancomat\n", id)
	default:
		fmt.Printf("[Cliente %d]: error\n", id)
		done <- true
		return
	}

	spesa := <-richiesta.ack

	if spesa >= 0 {
		fmt.Printf("[Cliente %d]: ho speso %d per 1kg di patate\n", id, spesa)
	} else {
		fmt.Printf("[Cliente %d]: error\n", id)
	}

	done <- true
	return
}

func mercato() {
	fmt.Printf("[Mercato]: start\n")

	var merce int = 0
	var cassa int = 50
	var conto int = 150
	var running bool = true

	for {
		select {
		case richiesta := <-when((running && merce > 0 && (cassa < X) || (len(acquirente_bancomat) == 0)), acquirente_contanti):
			merce--
			cassa += PAP
			richiesta.ack <- PAP
			fmt.Printf("[Mercato]: cliente ha pagato in contanti. Merce nel deposito: %d. Soldi in cassa: %d. Soldi in conto: %d\n", merce, cassa, conto)
		case richiesta := <-when((running && merce > 0 && (cassa > X) || (len(acquirente_contanti) == 0)), acquirente_bancomat):
			merce--
			conto += PAP
			richiesta.ack <- PAP
			fmt.Printf("[Mercato]: cliente ha pagato in bancomat. Merce nel deposito: %d. Soldi in cassa: %d. Soldi in conto: %d\n", merce, cassa, conto)
		case richiesta := <-when((running && merce+10 < N && cassa >= (PVP*10) && (cassa > X) || (len(fornitore_bonifico) == 0)), fornitore_contanti):
			merce += 10
			cassa -= PVP * 10
			richiesta.ack <- PVP * 10
			fmt.Printf("[Mercato]: fornitore è stato pagato in contanti. Merce nel deposito: %d. Soldi in cassa: %d. Soldi in conto: %d\n", merce, cassa, conto)
		case richiesta := <-when((running && merce+10 < N && conto >= (PVP*10) && (cassa < X) || (len(fornitore_contanti) == 0)), fornitore_bonifico):
			merce += 10
			conto -= PVP * 10
			richiesta.ack <- PVP * 10
			fmt.Printf("[Mercato]: fornitore è stato pagato in bancomat. Merce nel deposito: %d. Soldi in cassa: %d. Soldi in conto: %d\n", merce, cassa, conto)
		case richiesta := <-when(!running, acquirente_contanti):
			richiesta.ack <- -1
		case richiesta := <-when(!running, acquirente_bancomat):
			richiesta.ack <- -1
		case richiesta := <-when(!running, fornitore_contanti):
			richiesta.ack <- -1
		case richiesta := <-when(!running, fornitore_bonifico):
			richiesta.ack <- -1
		case <-termina:
			if running {
				running = false
				fmt.Print("[Mercato]: inizio terminazione\n")
			} else {
				fmt.Printf("[Mercato]: termina\n")
				done <- true
				return
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////
// Main
/////////////////////////////////////////////////////////////////////
func main() {
	rand.Seed(time.Now().UTC().UnixNano())

	var numAcquirenti int = 110
	var numFornitori int = 50

	done = make(chan bool, numAcquirenti+numFornitori)

	go mercato()

	for i := 0; i < numAcquirenti; i++ {
		go cliente(i)
	}

	for i := 0; i < numFornitori; i++ {
		go agricoltore(i)
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
	fmt.Println("[Main]: terminated")
}
