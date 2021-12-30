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
const MAX = 3
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
var entrata_spazzaneve = make(chan Richiesta, MAXBUFF)
var entrata_spargisale = make(chan Richiesta, MAXBUFF)
var entrata_camion = make(chan Richiesta, MAXBUFF)
var uscita_spazzaneve = make(chan Richiesta)
var uscita_spargisale = make(chan Richiesta)
var uscita_camion = make(chan Richiesta)

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

/////////////////////////////////////////////////////////////////////
// Goroutine
/////////////////////////////////////////////////////////////////////
func spazzaneve(id int) {
	richiesta := Richiesta{id, make(chan int)}

	entrata_spazzaneve <- richiesta
	<-richiesta.ack

	fmt.Printf("[SPAZZANEVE %d]: entrato\n", id)
	time.Sleep(time.Duration(rand.Intn(5)+1) * time.Second)

	uscita_spazzaneve <- richiesta
	<-richiesta.ack

	fmt.Printf("[SPAZZANEVE %d]: uscito\n", id)

	done <- true
}

func spargisale(id int) {
	richiesta := Richiesta{id, make(chan int)}

	entrata_spargisale <- richiesta
	<-richiesta.ack

	fmt.Printf("[SPARGISALE %d]: entrato per rifornirsi\n", id)
	time.Sleep(time.Duration(rand.Intn(5)+1) * time.Second)

	uscita_spargisale <- richiesta
	<-richiesta.ack

	fmt.Printf("[SPARGISALE %d]: uscito\n", id)

	done <- true
}

func camion(id int) {
	richiesta := Richiesta{id, make(chan int)}

	time.Sleep(time.Duration(rand.Intn(7)+1) * time.Second)

	entrata_camion <- richiesta
	<-richiesta.ack

	fmt.Printf("[CAMION %d]: entrato per rifornire il silos\n", id)
	time.Sleep(time.Duration(rand.Intn(5)+1) * time.Second)

	uscita_camion <- richiesta
	<-richiesta.ack

	fmt.Printf("[CAMION %d]: uscito\n", id)

	done <- true
}

func DMN() {
	var silos int = K
	var mezzi int = 0

	for {
		select {
		case richiesta := <-when((mezzi < MAX), entrata_spazzaneve):
			mezzi++
			richiesta.ack <- 1
			fmt.Printf("[DMN]: spazzaneve %d entrato. Mezzi dentro: %d, Mq nel silos: %d\n", richiesta.id, mezzi, silos)
		case richiesta := <-when((mezzi < MAX && len(entrata_spazzaneve) == 0 && len(entrata_camion) == 0 && silos > 0), entrata_spargisale):
			mezzi++
			silos--
			richiesta.ack <- 1
			fmt.Printf("[DMN]: spargisale %d entrato per rifornirsi. Mezzi dentro: %d, Mq nel silos: %d\n", richiesta.id, mezzi, silos)
		case richiesta := <-when((mezzi < MAX && len(entrata_spazzaneve) == 0 && silos < 10), entrata_camion):
			mezzi++
			silos++
			richiesta.ack <- 1
			fmt.Printf("[DMN]: camion %d entrato per rifornire il silos. Mezzi dentro: %d, Mq nel silos: %d\n", richiesta.id, mezzi, silos)
		case richiesta := <-uscita_spazzaneve:
			mezzi--
			richiesta.ack <- 1
		case richiesta := <-uscita_spargisale:
			mezzi--
			richiesta.ack <- 1
		case richiesta := <-uscita_camion:
			mezzi--
			richiesta.ack <- 1
		case <-termina:
			fmt.Println("\n[DMN]: Fine!")
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

	for i := 0; i < 10; i++ {
		go spazzaneve(i)
	}

	for i := 10; i < 20; i++ {
		go spargisale(i)
	}

	for i := 20; i < 25; i++ {
		go camion(i)
	}

	go DMN()

	// join
	for i := 0; i < 25; i++ {
		<-done
	}

	termina <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
