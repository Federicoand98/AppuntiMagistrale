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
const MAX = 20
const TOT = 50
const M = 8

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
var deposito_biscotto = make(chan Richiesta, MAXBUFF)
var prelievo_biscotto = make(chan Richiesta, MAXBUFF)
var richiesta_rifornimento = make(chan Richiesta, MAXBUFF)

/////////////////////////////////////////////////////////////////////
// Join Goroutine
/////////////////////////////////////////////////////////////////////
var done = make(chan bool)
var termina_alimentatore = make(chan bool)
var termina_operaio = make(chan bool)

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
func MB() {
	richiesta := Richiesta{0, make(chan int)} // se si usa struttura
	fmt.Printf("[MB]: avviata\n")

	for i := 0; i < TOT*2; i++ {
		fmt.Printf("[MB]: preparazione biscotto\n")

		sleepRandTime(3)

		deposito_biscotto <- richiesta
		<-richiesta.ack
		fmt.Printf("[MB]: depositato un biscotto\n")
	}

	fmt.Printf("[MB]: ho finito la produzione e il deposito, termino\n")
	done <- true
	return
}

func MG() {
	richiesta := Richiesta{1, make(chan int)}
	var gelato int = 0

	fmt.Printf("[MG]: avviato\n")

	for i := 0; i < TOT; i++ {
		for j := 0; j < 2; j++ {
			prelievo_biscotto <- richiesta
			<-richiesta.ack
			fmt.Printf("[MG]: prelevato un biscotto\n")
		}

		sleepRandTime(2)

		if gelato == 0 {
			richiesta_rifornimento <- richiesta
			res := <-richiesta.ack
			gelato += res
			fmt.Printf("[MG]: gelato finito, richiesto rifornimento\n")
		}

		sleepRandTime(3)

		gelato--
		fmt.Printf("[MG]: prodotto gelato numero %d\n", i+1)
	}

	fmt.Printf("[MG]: ho terminato la produzione di gelati, termino\n")
	done <- true
	return
}

func operaio() {
	fmt.Printf("[Operaio]: avviato\n")

	for {
		select {
		case richiesta := <-richiesta_rifornimento:
			richiesta.ack <- M
			fmt.Printf("[Operaio]: rifornito MG con %d gelati\n", M)
		case <-termina_operaio:
			fmt.Printf("[Operaio]: termino\n")
			done <- true
			return
		}
	}
}

func alimentatore() {
	var n_biscotti int = 0

	fmt.Printf("[Alimentatore]: avviato\n")

	for {
		select {
		case richiesta := <-when(n_biscotti < MAX && ((n_biscotti < MAX/2) || (n_biscotti >= MAX/2 && len(prelievo_biscotto) == 0)), deposito_biscotto):
			n_biscotti++
			richiesta.ack <- 1
			fmt.Printf("[Alimentatore]: MB ha depositato un biscotto. Num Biscotti: %d\n", n_biscotti)
		case richiesta := <-when(n_biscotti > 0 && ((n_biscotti > MAX/2) || (n_biscotti <= MAX/2 && len(deposito_biscotto) == 0)), prelievo_biscotto):
			n_biscotti--
			richiesta.ack <- 1
			fmt.Printf("[Alimentatore]: MG ha prelevato un biscotto. Num Biscotti: %d\n", n_biscotti)
		case <-termina_alimentatore:
			fmt.Println("[Alimentatore]: termino")
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

	go MB()
	go MG()
	go operaio()
	go alimentatore()

	<-done
	<-done
	termina_operaio <- true
	<-done
	termina_alimentatore <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
