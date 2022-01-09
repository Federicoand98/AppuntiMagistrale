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
const N = 10
const DEBITORE = 0
const CREDITORE = 1

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
var entrata_1_sala = make(chan Richiesta, MAXBUFF)
var entrata_2_sala = make(chan Richiesta, MAXBUFF)
var uscita_sala = make(chan Richiesta, MAXBUFF)
var cassa_debitori = make(chan Richiesta, MAXBUFF)
var cassa_creditori = make(chan Richiesta, MAXBUFF)

/////////////////////////////////////////////////////////////////////
// Join Goroutine
/////////////////////////////////////////////////////////////////////
var done = make(chan bool)
var termina_sala = make(chan bool)
var termina_cassa = make(chan bool)

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
func visitatore(id int, entrata int) {
	richiesta := Richiesta{id, make(chan int)} // se si usa struttura

	if entrata == 0 {
		entrata_1_sala <- richiesta
		<-richiesta.ack
		fmt.Printf("[Visitatore %d]: entrato in sala attraverso l'entrata 1\n", id)
	} else if entrata == 1 {
		entrata_2_sala <- richiesta
		<-richiesta.ack
		fmt.Printf("[Visitatore %d]: entrato in sala attraverso l'entrata 2\n", id)
	}

	sleepRandTime(5)

	uscita_sala <- richiesta
	<-richiesta.ack

	giocata := rand.Intn(2)

	if giocata == DEBITORE {
		fmt.Printf("[Visitatore %d]: al termine delle partite è risultato debitore\n", id)
		cassa_debitori <- richiesta
		<-richiesta.ack
		fmt.Printf("[Visitatore %d]: ha pagato il suo debito\n", id)
	} else if giocata == CREDITORE {
		fmt.Printf("[Visitatore %d]: al termine delle partite è risultato creditore\n", id)
		cassa_creditori <- richiesta
		<-richiesta.ack
		fmt.Printf("[Visitatore %d]: ha riscosso il suo credito\n", id)
	}

	done <- true
}

func sala() {
	var visitatori int = 0

	for {
		select {
		case richiesta := <-when(visitatori < N && ((len(entrata_1_sala) < len(entrata_2_sala)) || (len(entrata_1_sala) >= len(entrata_2_sala) && len(entrata_2_sala) == 0)), entrata_1_sala):
			visitatori++
			richiesta.ack <- 1
			fmt.Printf("[Sala]: entrato visitatore dall'entrata 1. Num Visitatori dentro: %d\n", visitatori)
		case richiesta := <-when(visitatori < N && ((len(entrata_2_sala) < len(entrata_1_sala)) || (len(entrata_2_sala) >= len(entrata_1_sala) && len(entrata_1_sala) == 0)), entrata_2_sala):
			visitatori++
			richiesta.ack <- 1
			fmt.Printf("[Sala]: entrato visitatore dall'entrata 2. Num Visitatori dentro: %d\n", visitatori)
		case richiesta := <-when(true, uscita_sala):
			visitatori--
			richiesta.ack <- 1
			fmt.Printf("[Sala]: uscito visitatore. Num Visitatori dentro: %d\n", visitatori)
		case <-termina_sala:
			fmt.Printf("[Sala]: termino\n")
			done <- true
			return
		}
	}
}

func cassa() {
	for {
		select {
		case richiesta := <-when(len(cassa_debitori) == 0, cassa_creditori):
			richiesta.ack <- 1
			fmt.Printf("[Cassa]: visitatore %d creditore ha ricevuto il suo credito\n", richiesta.id)
		case richiesta := <-when(true, cassa_debitori):
			richiesta.ack <- 1
			fmt.Printf("[Cassa]: visitatore %d debitore ha pagato il suo debito\n", richiesta.id)
		case <-termina_cassa:
			fmt.Println("[Cassa]: termino")
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

	var V1 int = 50

	go cassa()
	go sala()

	for i := 0; i < V1; i++ {
		go visitatore(i, rand.Intn(2))
	}

	// join
	for i := 0; i < V1; i++ {
		<-done
	}

	termina_sala <- true
	<-done
	termina_cassa <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
