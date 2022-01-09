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
const N = 5
const OVER = 0
const UNDER = 1

var tipoCliente = [2]string{"Over 70", "Under 70"}

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
var richiesta_sportello [2]chan Richiesta
var libera_sportello [2]chan Richiesta
var entra_caveau [2]chan Richiesta
var libera_caveau [2]chan Richiesta

/////////////////////////////////////////////////////////////////////
// Join Goroutine
/////////////////////////////////////////////////////////////////////
var done = make(chan bool)
var termina_caveau = make(chan bool)
var termina_sportelli = make(chan bool)

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
func cliente(id int, tipo int) {
	richiesta := Richiesta{id, make(chan int)} // se si usa struttura
	fmt.Printf("[Cliente %d %s]: avviato\n", id, tipoCliente[tipo])

	richiesta_sportello[tipo] <- richiesta
	<-richiesta.ack

	libera_sportello[tipo] <- richiesta
	<-richiesta.ack
	fmt.Printf("[Cliente %d %s]: presentata richiesta della chiave allo sportello, mi dirigo al caveau\n", id, tipoCliente[tipo])

	sleepRandTime(3)

	entra_caveau[tipo] <- richiesta
	<-richiesta.ack
	fmt.Printf("[Cliente %d %s]: sono entrato nel caveau\n", id, tipoCliente[tipo])

	sleepRandTime(5)

	libera_caveau[tipo] <- richiesta
	<-richiesta.ack
	fmt.Printf("[Cliente %d %s]: uscito dal caveau\n", id, tipoCliente[tipo])

	sleepRandTime(3)

	richiesta_sportello[tipo] <- richiesta
	<-richiesta.ack

	libera_sportello[tipo] <- richiesta
	<-richiesta.ack
	fmt.Printf("[Cliente %d %s]: consegnata chiave allo sportello, me ne vado\n", id, tipoCliente[tipo])

	done <- true
}

func sportelli() {
	var liberi int = N

	for {
		select {
		case richiesta := <-when(liberi > 0, richiesta_sportello[OVER]):
			liberi--
			richiesta.ack <- 1
			fmt.Printf("[Sportelli]: entrato cliente over. Num Sportelli Liberi: %d\n", liberi)
		case richiesta := <-when(liberi > 0 && len(richiesta_sportello[OVER]) == 0, richiesta_sportello[UNDER]):
			liberi--
			richiesta.ack <- 1
			fmt.Printf("[Sportelli]: entrato cliente under. Num Sportelli Liberi: %d\n", liberi)
		case richiesta := <-when(true, libera_sportello[OVER]):
			liberi++
			richiesta.ack <- 1
			fmt.Printf("[Sportelli]: uscito cliente over. Num Sportelli Liberi: %d\n", liberi)
		case richiesta := <-when(true, libera_sportello[UNDER]):
			liberi++
			richiesta.ack <- 1
			fmt.Printf("[Sportelli]: uscito cliente under. Num Sportelli Liberi: %d\n", liberi)
		case <-termina_sportelli:
			fmt.Printf("[Sportelli]: termine\n")
			done <- true
			return
		}
	}
}

func caveau() {
	var libero bool = true
	var n_over int = rand.Intn(10)
	var n_under int = rand.Intn(10)

	for {
		select {
		case richiesta := <-when(libero && ((n_over < n_under) || (n_over >= n_under && len(entra_caveau[UNDER]) == 0)), entra_caveau[OVER]):
			libero = false
			n_over++
			richiesta.ack <- 1
			fmt.Printf("[Caveau]: entrato over. Storico Over: %d, Storico Under: %d\n", n_over, n_under)
		case richiesta := <-when(libero && ((n_under < n_over) || (n_under >= n_over && len(entra_caveau[OVER]) == 0)), entra_caveau[UNDER]):
			libero = false
			n_over++
			richiesta.ack <- 1
			fmt.Printf("[Caveau]: entrato under. Storico Over: %d, Storico Under: %d\n", n_over, n_under)
		case richiesta := <-when(true, libera_caveau[OVER]):
			libero = true
			richiesta.ack <- 1
			fmt.Printf("[Caveau]: uscito over. Storico Over: %d, Storico Under: %d\n", n_over, n_under)
		case richiesta := <-when(true, libera_caveau[UNDER]):
			libero = true
			richiesta.ack <- 1
			fmt.Printf("[Caveau]: uscito under. Storico Over: %d, Storico Under: %d\n", n_over, n_under)
		case <-termina_caveau:
			fmt.Println("\n\n[Caveau]: Fine!")
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

	//inizializzazione canali se non si usa struttura dati
	for i := 0; i < 2; i++ {
		richiesta_sportello[i] = make(chan Richiesta, MAXBUFF)
		libera_sportello[i] = make(chan Richiesta, MAXBUFF)
		entra_caveau[i] = make(chan Richiesta, MAXBUFF)
		libera_caveau[i] = make(chan Richiesta, MAXBUFF)
	}

	go sportelli()
	go caveau()

	for i := 0; i < V1; i++ {
		go cliente(i, rand.Intn(2))
	}

	// join
	for i := 0; i < V1; i++ {
		<-done
	}

	termina_sportelli <- true
	<-done
	termina_caveau <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
