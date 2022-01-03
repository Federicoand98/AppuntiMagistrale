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
const N = 20
const CONTANTI = 0
const CARTA = 1
const PIADINA = 0
const CRESCIONE = 1

var tipoCibo = [2]string{"Piadina", "Crescione"}

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
var cliente_ordina_contanti = make(chan Richiesta, MAXBUFF)
var cliente_ordina_bancomat = make(chan Richiesta, MAXBUFF)
var richiesta_piadina = make(chan Richiesta, MAXBUFF)
var richiesta_crescione = make(chan Richiesta, MAXBUFF)
var preparazione_piadina = make(chan Richiesta, MAXBUFF)
var preparazione_crescione = make(chan Richiesta, MAXBUFF)

/////////////////////////////////////////////////////////////////////
// Join Goroutine
/////////////////////////////////////////////////////////////////////
var done = make(chan bool)
var termina_cassiere = make(chan bool)
var termina_addetto = make(chan bool)

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
func cliente(id int) {
	richiesta := Richiesta{id, make(chan int)}

	pagamento := rand.Intn(2)

	switch pagamento {
	case CONTANTI:
		cliente_ordina_contanti <- richiesta
		fmt.Printf("[Cliente %d]: ordina in contanti\n", id)
	case CARTA:
		cliente_ordina_bancomat <- richiesta
		fmt.Printf("[Cliente %d]: ordina in carta\n", id)
	default:
		fmt.Printf("[Cliente %d]: errore.\n", id)
		done <- true
		return
	}

	<-richiesta.ack

	sleepRandTime(3)

	num_ordinazioni := rand.Intn(3)

	for i := 0; i < num_ordinazioni; i++ {
		tipo := rand.Intn(2)

		if tipo == PIADINA {
			richiesta_piadina <- richiesta
		} else if tipo == CRESCIONE {
			richiesta_crescione <- richiesta
		}

		esito := <-richiesta.ack

		if esito == -1 {
			fmt.Printf("[Cliente %d]: termino.\n", id)
			done <- true
			return
		}

		fmt.Printf("[Cliente %d]: ordine %d di %d: %s\n", id, i, num_ordinazioni, tipoCibo[tipo])
	}

	done <- true
	return
}

func cuoco(id int) {
	richiesta := Richiesta{id, make(chan int)}

	for {
		tipo := rand.Intn(2)

		if tipo == PIADINA {
			preparazione_piadina <- richiesta
		} else if tipo == CRESCIONE {
			preparazione_crescione <- richiesta
		}

		esito := <-richiesta.ack

		if esito == -1 {
			fmt.Printf("[Cuoco]: termino\n")
			done <- true
			return
		}

		fmt.Printf("[Cuoco]: %s preparato.\n", tipoCibo[tipo])

		sleepRandTime(4)
	}
}

func addetto() {
	var running bool = true
	var n_piadine int = 0
	var n_crescioni int = 0

	for {
		select {
		case richiesta := <-when(running && n_piadine > 0 && len(richiesta_crescione) == 0, richiesta_piadina):
			n_piadine--
			richiesta.ack <- 1
			fmt.Printf("[Addetto]: consegnata piadina. Num Piadine: %d. Num Crescioni: %d. Totale: %d. Max: %d\n", n_piadine, n_crescioni, n_crescioni+n_piadine, N)
		case richiesta := <-when(running && n_crescioni > 0, richiesta_crescione):
			n_crescioni--
			richiesta.ack <- 1
			fmt.Printf("[Addetto]: consegnato crescione. Num Piadine: %d. Num Crescioni: %d. Totale: %d. Max: %d\n", n_piadine, n_crescioni, n_crescioni+n_piadine, N)
		case richiesta := <-when(running && n_crescioni+n_piadine < N, preparazione_piadina):
			n_piadine++
			richiesta.ack <- 1
			fmt.Printf("[Addetto]: preparata piadina. Num Piadine: %d. Num Crescioni: %d. Totale: %d. Max: %d\n", n_piadine, n_crescioni, n_crescioni+n_piadine, N)
		case richiesta := <-when(running && n_crescioni+n_piadine < N, preparazione_crescione):
			n_crescioni++
			richiesta.ack <- 1
			fmt.Printf("[Addetto]: preparato crescione. Num Piadine: %d. Num Crescioni: %d. Totale: %d. Max: %d\n", n_piadine, n_crescioni, n_crescioni+n_piadine, N)
		case richiesta := <-when(!running, richiesta_piadina):
			richiesta.ack <- -1
		case richiesta := <-when(!running, richiesta_crescione):
			richiesta.ack <- -1
		case richiesta := <-when(!running, preparazione_piadina):
			richiesta.ack <- -1
		case richiesta := <-when(!running, preparazione_crescione):
			richiesta.ack <- -1
		case <-termina_addetto:
			if running {
				fmt.Println("[Addetto]: start termination")
				running = false
			} else {
				fmt.Println("[Addetto]: terminato")
				done <- true
				return
			}
		}
	}
}

func cassiere() {
	for {
		select {
		case richiesta := <-when(len(cliente_ordina_bancomat) == 0, cliente_ordina_contanti):
			richiesta.ack <- 1
			fmt.Printf("[Cassiere]: cliente %d ha pagato in contanti\n", richiesta.id)
		case richiesta := <-when(true, cliente_ordina_bancomat):
			richiesta.ack <- 1
			fmt.Printf("[Cassiere]: cliente %d ha pagato in carta\n", richiesta.id)
		case <-termina_cassiere:
			fmt.Printf("[Cassiere]: termino\n")
			done <- true
			return
		}
	}
}

/////////////////////////////////////////////////////////////////////
// Main
/////////////////////////////////////////////////////////////////////
func main() {
	rand.Seed(time.Now().Unix())

	var n_clienti int = 20

	go addetto()
	go cassiere()
	go cuoco(0)

	for i := 1; i <= n_clienti; i++ {
		go cliente(i)
	}

	for i := 1; i <= n_clienti; i++ {
		<-done
	}

	termina_addetto <- true
	<-done
	<-done
	termina_cassiere <- true
	<-done

	fmt.Printf("\n[Main]: fine.\n")
}
