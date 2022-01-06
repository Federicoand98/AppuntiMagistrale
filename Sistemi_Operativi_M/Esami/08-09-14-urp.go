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
const N = 15
const TASSE = 0
const SCUOLA = 1
const EDILIZIA = 2

var tipologia = [3]string{"Tasse", "Scuola", "Edilizia"}

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
var richiesta_colloquio [3]chan Richiesta
var fine_colloquio [3]chan Richiesta

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

/////////////////////////////////////////////////////////////////////
// Goroutine
/////////////////////////////////////////////////////////////////////
func utente(id int, tipo int) {
	richiesta := Richiesta{id, make(chan int)} // se si usa struttura

	richiesta_colloquio[tipo] <- richiesta
	<-richiesta.ack
	fmt.Printf("[Utente %d]: richiesto colloquio per %s\n", id, tipologia[tipo])

	sleepRandTime(7)

	fine_colloquio[tipo] <- richiesta
	<-richiesta.ack
	fmt.Printf("[Utente %d]: fine colloquio per %s\n", id, tipologia[tipo])

	done <- true
}

func URP() {
	var operatori int = N

	for {
		select {
		case richiesta := <-when(operatori > 0, richiesta_colloquio[TASSE]):
			operatori--
			richiesta.ack <- 1
			fmt.Printf("[URP]: utente %d richiesto tasse. Num operatori liberi: %d\n", richiesta.id, operatori)
		case richiesta := <-when(operatori > 0 && len(richiesta_colloquio[TASSE]) == 0 && ((len(richiesta_colloquio[SCUOLA]) > len(richiesta_colloquio[EDILIZIA])) || (len(richiesta_colloquio[EDILIZIA]) == 0)), richiesta_colloquio[SCUOLA]):
			operatori--
			richiesta.ack <- 1
			fmt.Printf("[URP]: utente %d richiesto scuola. Num operatori liberi: %d\n", richiesta.id, operatori)
		case richiesta := <-when(operatori > 0 && len(richiesta_colloquio[TASSE]) == 0 && ((len(richiesta_colloquio[EDILIZIA]) > len(richiesta_colloquio[SCUOLA])) || (len(richiesta_colloquio[SCUOLA]) == 0)), richiesta_colloquio[EDILIZIA]):
			operatori--
			richiesta.ack <- 1
			fmt.Printf("[URP]: utente %d richiesto edilizia. Num operatori liberi: %d\n", richiesta.id, operatori)
		case richiesta := <-when(true, fine_colloquio[TASSE]):
			operatori++
			richiesta.ack <- 1
			fmt.Printf("[URP]: utente %d fine tasse. Num operatori liberi: %d\n", richiesta.id, operatori)
		case richiesta := <-when(true, fine_colloquio[SCUOLA]):
			operatori++
			richiesta.ack <- 1
			fmt.Printf("[URP]: utente %d fine scuola. Num operatori liberi: %d\n", richiesta.id, operatori)
		case richiesta := <-when(true, fine_colloquio[EDILIZIA]):
			operatori++
			richiesta.ack <- 1
			fmt.Printf("[URP]: utente %d fine edilizia. Num operatori liberi: %d\n", richiesta.id, operatori)
		case <-termina:
			fmt.Println("\n\n[URP]: Fine!")
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

	var V1 int = 40

	//inizializzazione canali se non si usa struttura dati
	for i := 0; i < 3; i++ {
		richiesta_colloquio[i] = make(chan Richiesta, MAXBUFF)
		fine_colloquio[i] = make(chan Richiesta, MAXBUFF)
	}

	go URP()

	for i := 0; i < V1; i++ {
		go utente(i, rand.Intn(3))
	}

	// join
	for i := 0; i < V1; i++ {
		<-done
	}

	termina <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
