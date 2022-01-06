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
const MAX = 5
const IN = 0
const OUT = 1

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
var entrata_direzione_in = make(chan Richiesta, MAXBUFF)
var entrata_direzione_out = make(chan Richiesta, MAXBUFF)
var uscita_direzione_in = make(chan Richiesta, MAXBUFF)
var uscita_direzione_out = make(chan Richiesta, MAXBUFF)
var richiesta_chiusura = make(chan Richiesta, MAXBUFF)
var chiusura = make(chan Richiesta, MAXBUFF)
var apertura = make(chan Richiesta, MAXBUFF)

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
func viaggiatore(id int, tipo int) {
	richiesta := Richiesta{id, make(chan int)}

	if tipo == IN {
		fmt.Printf("[Viaggiatore %d]: avviato.\n", id)

		entrata_direzione_in <- richiesta
		<-richiesta.ack
		fmt.Printf("[Viaggiatore %d]: entrato in direzione IN\n", id)

		sleepRandTime(10)

		uscita_direzione_in <- richiesta
		<-richiesta.ack
		fmt.Printf("[Viaggiatore %d]: uscito in direzione IN\n", id)
	} else if tipo == OUT {
		fmt.Printf("[Viaggiatore %d]: avviato.\n", id)

		entrata_direzione_out <- richiesta
		<-richiesta.ack
		fmt.Printf("[Viaggiatore %d]: entrato in direzione OUT\n", id)

		sleepRandTime(10)

		uscita_direzione_out <- richiesta
		<-richiesta.ack
		fmt.Printf("[Viaggiatore %d]: uscito in direzione OUT\n", id)
	}

	done <- true
}

func addetto() {
	var ris int
	richiesta := Richiesta{100, make(chan int)}
	fmt.Printf("[Addetto]: avviato\n")

	for {
		sleepRandTime(10)

		fmt.Printf("[Addetto]: rilevato un problema, chiedo la chiusura\n")

		richiesta_chiusura <- richiesta
		ris = <-richiesta.ack

		if ris == -1 {
			fmt.Printf("[Addetto]: termino\n")
			done <- true
			return
		}

		chiusura <- richiesta
		ris = <-richiesta.ack
		fmt.Printf("[Addetto]: molo chiuso\n")

		if ris == -1 {
			fmt.Printf("[Addetto]: termino\n")
			done <- true
			return
		}

		sleepRandTime(10)

		apertura <- richiesta
		ris = <-richiesta.ack

		if ris == -1 {
			fmt.Printf("[Addetto]: termino\n")
			done <- true
			return
		}

		fmt.Printf("[Addetto]: molo riaperto\n")
	}
}

func passerella() {
	var running bool = true
	var vuole_chiudere bool = false
	var aperto bool = true
	var num int = 0
	var num_in int = 0
	var num_out int = 0

	for {
		select {
		case richiesta := <-when(running && aperto && !vuole_chiudere && num < MAX && num_out == 0 && len(entrata_direzione_out) == 0, entrata_direzione_in):
			num++
			num_in++
			richiesta.ack <- 1
			fmt.Printf("[Passerella]: entrato sul ponte utente %d in direzione IN\n", richiesta.id)
		case richiesta := <-when(running && aperto && !vuole_chiudere && num < MAX && num_in == 0, entrata_direzione_out):
			num++
			num_out++
			richiesta.ack <- 1
			fmt.Printf("[Passerella]: entrato sul ponte utente %d in direzione OUT\n", richiesta.id)
		case richiesta := <-when(running && aperto, uscita_direzione_in):
			num--
			num_in--
			richiesta.ack <- 1
			fmt.Printf("[Passerella]: uscito dal ponte utente %d in direzione IN\n", richiesta.id)
		case richiesta := <-when(running && aperto, uscita_direzione_out):
			num--
			num_out--
			richiesta.ack <- 1
			fmt.Printf("[Passerella]: uscito dal ponte utente %d in direzione OUT\n", richiesta.id)
		case richiesta := <-when(running && aperto, richiesta_chiusura):
			vuole_chiudere = true
			richiesta.ack <- 1
			fmt.Printf("[Passerella]: richiesta di chiusura da parte dell'addetto\n")
		case richiesta := <-when(running && aperto && num == 0, chiusura):
			aperto = false
			richiesta.ack <- 1
			fmt.Printf("[Passerella]: ponte chiuso\n")
		case richiesta := <-when(running && !aperto, apertura):
			aperto = true
			vuole_chiudere = false
			richiesta.ack <- 1
			fmt.Printf("[Passerella]: ponte aperto\n")
		case richiesta := <-when(!running, richiesta_chiusura):
			richiesta.ack <- -1
		case richiesta := <-when(!running, chiusura):
			richiesta.ack <- -1
		case richiesta := <-when(!running, apertura):
			richiesta.ack <- -1
		case <-termina:
			if running {
				fmt.Println("[Passerella]: start termination")
				running = false
			} else {
				fmt.Println("[Passerella]: terminato")
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

	var V1 int = 40

	go passerella()
	go addetto()

	for i := 0; i < V1; i++ {
		go viaggiatore(i, rand.Intn(2))
	}

	// join
	for i := 0; i < V1; i++ {
		<-done
	}

	termina <- true
	<-done
	termina <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
