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
const M = 8
const MAX_P = 80
const MAX_S = 100

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
var consegna_OO = make(chan Richiesta, MAXBUFF)
var ritira_OO = make(chan Richiesta, MAXBUFF)
var ritira_AL = make(chan Richiesta, MAXBUFF)
var deposita_AL = make(chan Richiesta, MAXBUFF)

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
func operatore(id int) {
	fmt.Printf("[Operatore %d]: avviato\n", id)

	richiesta := Richiesta{id: id, ack: make(chan int)}

	consegna_OO <- richiesta
	<-richiesta.ack

	fmt.Printf("[Operatore %d]: consegnate %d lenzuola sporche.\n", id, N)

	sleepRandTime(5)

	ritira_OO <- richiesta
	<-richiesta.ack

	fmt.Printf("[Operatore %d]: ritirate %d lenzuola pulite\n", id, N)

	done <- true
}

// alternativa -> il testo dell'esame è poco chiaro
/*
func operatore(id int) {
	fmt.Printf("[Operatore %d]: avviato\n", id)

	richiesta := Richiesta{id: id, qty: rand.Intn(100), ack: make(chan int)}

	if richiesta.qty < MAX_S/2 {
		consegna_OO_under <- richiesta
	} else {
		consegna_OO_over <- richiesta
	}

	<-richiesta.ack

	fmt.Printf("[Operatore %d]: consegnate %d lenzuola sporche.\n", id, richiesta.qty)

	sleepRandTime(5)

	ritira_OO <- richiesta
	<-richiesta.ack

	fmt.Printf("[Operatore %d]: ritirate %d lenzuola pulite\n", id, N)

	done <- true
}
*/

func addetto(id int) {
	fmt.Printf("[Addetto %d]: avviato\n", id)

	richiesta := Richiesta{id: id, ack: make(chan int)}

	ritira_AL <- richiesta
	<-richiesta.ack

	fmt.Printf("[Addetto %d]: ritirate %d lenzuola sporche.\n", id, M)

	sleepRandTime(7)

	deposita_AL <- richiesta
	<-richiesta.ack

	fmt.Printf("[Addetto %d]: depositate %d lenzuola pulite.\n", id, M)

	done <- true
}

func deposito() {
	fmt.Printf("[Deposito]: avviato\n")

	var num_s int = 0
	var num_p int = 0

	for {
		select {
		case richiesta := <-when((num_s+N < MAX_S) && (num_p > num_s || (len(deposita_AL) == 0 && len(ritira_AL) == 0)), consegna_OO):
			num_s += N
			richiesta.ack <- 1
			fmt.Printf("[Deposito]: depositate da OO %d sporche. Num_s: %d. Num_p: %d.\n", N, num_s, num_p)
		case richiesta := <-when((num_p >= N) && (num_p > num_s || (len(ritira_AL) == 0 && len(deposita_AL) == 0)), ritira_OO):
			num_p -= N
			richiesta.ack <- 1
			fmt.Printf("[Deposito]: ritirate da OO %d pulite. Num_s: %d. Num_p: %d.\n", N, num_s, num_p)
		case richiesta := <-when((num_p+M < MAX_P) && (num_s > num_p || (len(consegna_OO) == 0 && len(ritira_OO) == 0)), deposita_AL):
			num_p += M
			richiesta.ack <- 1
			fmt.Printf("[Deposito]: depositate da AL %d pulite. Num_s: %d. Num_p: %d.\n", M, num_s, num_p)
		case richiesta := <-when((num_s >= M) && (num_s > num_p || (len(consegna_OO) == 0 && len(ritira_OO) == 0)), ritira_AL):
			num_s -= M
			richiesta.ack <- 1
			fmt.Printf("[Deposito]: ritirate da AL %d sporche. Num_s: %d. Num_p: %d.\n", M, num_s, num_p)
		case <-termina:
			fmt.Println("\n[Deposito]: Fine!")
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

	var V1 int = 30
	var V2 int = 25

	go deposito()

	for i := 0; i < V1; i++ {
		go operatore(i)
	}

	for i := 0; i < V2; i++ {
		go addetto(i)
	}

	// join
	for i := 0; i < V1+V2; i++ {
		<-done
	}

	termina <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
