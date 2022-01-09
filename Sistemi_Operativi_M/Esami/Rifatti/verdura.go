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
const PAp = 20
const PVp = 3
const N = 100
const X = 50
const K = 10

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
var vendita_contanti = make(chan Richiesta, MAXBUFF)
var vendita_bonifico = make(chan Richiesta, MAXBUFF)
var acquisto_contanti = make(chan Richiesta, MAXBUFF)
var acquisto_bancomat = make(chan Richiesta, MAXBUFF)

/////////////////////////////////////////////////////////////////////
// Join Goroutine
/////////////////////////////////////////////////////////////////////
var done = make(chan bool)
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

func remove(slice []int, s int) []int {
	return append(slice[:s], slice[s+1:]...)
}

/////////////////////////////////////////////////////////////////////
// Goroutine
/////////////////////////////////////////////////////////////////////
func agricoltore(id int) {
	richiesta := Richiesta{id, make(chan int)} // se si usa struttura

	fmt.Printf("[Agricoltore %d]: avviato\n", id)

	for {
		pagamento := rand.Intn(2)

		if pagamento == CONTANTI {
			vendita_contanti <- richiesta
			fmt.Printf("[Agricoltore %d]: ha venduto 10kg di patate. Pagamento in contanti\n", id)
		} else if pagamento == BONIFICO {
			vendita_bonifico <- richiesta
			fmt.Printf("[Agricoltore %d]: ha venduto 10kg di patate. Pagamento in bonifico\n", id)
		}

		ris := <-richiesta.ack

		if ris == -1 {
			fmt.Printf("[Agricoltore %d]: termino\n", id)
			done <- true
			return
		}

		sleepRandTime(5)
	}
}

func cliente(id int) {
	richiesta := Richiesta{id, make(chan int)}

	fmt.Printf("[Cliente %d]: avviato\n", id)

	pagamento := rand.Intn(2)

	if pagamento == CONTANTI {
		acquisto_contanti <- richiesta
		fmt.Printf("[Cliente %d]: acquistato 1kg di patate. Pagamento in contanti", id)
	} else if pagamento == BANCOMAT {
		acquisto_bancomat <- richiesta
		fmt.Printf("[Cliente %d]: acquistato 1kg di patate. Pagamento in contanti", id)
	}

	ris := <-richiesta.ack

	if ris >= 0 {
		fmt.Printf("[Cliente %d]: ha speso %d per 1kg di patate.\n", id, ris)
	}

	done <- true
	return
}

func mercato() {
	var running bool = true
	var patate int = 0
	var cassa int = rand.Intn(50)

	for {
		select {
		case richiesta := <-when(running && cassa >= PVp*K && patate+K <= MAX && ((cassa >= X) || (cassa < X && len(vendita_bonifico) == 0)), vendita_contanti):
			cassa -= PVp * K
			patate += K
			richiesta.ack <- PVp * K
			fmt.Printf("[Mercato]: acquistati 10kg di patate da agricoltore pagamento contanti. Kg di Patate: %d, Soldi in Cassa: %d\n", patate, cassa)
		case richiesta := <-when(running && patate+K <= MAX && ((cassa < X) || (cassa >= X && len(vendita_contanti) == 0)), vendita_bonifico):
			patate += K
			richiesta.ack <- PVp * K
			fmt.Printf("[Mercato]: acquistati 10kg di patate da agricoltore pagamento bonifico. Kg di Patate: %d, Soldi in Cassa: %d\n", patate, cassa)
		case richiesta := <-when(running && patate > 0 && ((cassa < X) || (cassa >= X && len(acquisto_bancomat) == 0)), acquisto_contanti):
			cassa += PAp
			patate--
			richiesta.ack <- PAp
			fmt.Printf("[Mercato]: venduto 1kg di patate a cliente pagamento contanti. Kg di Patate: %d, Soldi in Cassa: %d\n", patate, cassa)
		case richiesta := <-when(running && patate > 0 && ((cassa >= X) || (cassa < X && len(acquisto_contanti) == 0)), acquisto_bancomat):
			patate--
			richiesta.ack <- PAp
			fmt.Printf("[Mercato]: venduto 1kg di patate a cliente pagamento bancomat. Kg di Patate: %d, Soldi in Cassa: %d\n", patate, cassa)
		case richiesta := <-when(!running, vendita_contanti):
			richiesta.ack <- -1
		case richiesta := <-when(!running, vendita_bonifico):
			richiesta.ack <- -1
		case richiesta := <-when(!running, acquisto_contanti):
			richiesta.ack <- -1
		case richiesta := <-when(!running, acquisto_bancomat):
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
	rand.Seed(time.Now().Unix())

	var clienti int = 100
	var agricoltori int = 25

	go mercato()

	for i := 0; i < agricoltori; i++ {
		go agricoltore(i)
	}

	for i := 0; i < clienti; i++ {
		go cliente(i)
	}

	for i := 0; i < clienti; i++ {
		<-done
	}

	<-termina

	for i := 0; i < agricoltori; i++ {
		<-done
	}

	termina <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
