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
	qty int
	ack chan int
}

/////////////////////////////////////////////////////////////////////
// Canali
/////////////////////////////////////////////////////////////////////
var consegna_OO_under = make(chan Richiesta, MAXBUFF)
var consegna_OO_over = make(chan Richiesta, MAXBUFF)
var ritira_OO = make(chan Richiesta, MAXBUFF)
var ritira_AL = make(chan Richiesta, MAXBUFF)
var deposita_AL_under = make(chan Richiesta, MAXBUFF)
var deposita_AL_over = make(chan Richiesta, MAXBUFF)

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

	richiesta := Richiesta{id: id, qty: rand.Intn(MAX_S), ack: make(chan int)}
	var ok bool = true

	for ok {
		if richiesta.qty < MAX_S/2 {
			consegna_OO_under <- richiesta
		} else {
			consegna_OO_over <- richiesta
		}

		ris := <-richiesta.ack

		if ris == 1 {
			fmt.Printf("[Operatore %d]: consegnate %d lenzuola sporche.\n", id, richiesta.qty)
			ok = false
		} else {
			fmt.Printf("[Operatore %d]: non sono riuscito a consegnare %d lenzuola sporche. Riprovo fra poco.\n", id, richiesta.qty)
			sleepRandTime(8)
		}
	}

	sleepRandTime(5)
	ok = true

	for ok {
		ritira_OO <- richiesta
		ris := <-richiesta.ack

		if ris == 1 {
			fmt.Printf("[Operatore %d]: ritirate %d lenzuola pulite\n", id, richiesta.qty)
			ok = false
		} else {
			fmt.Printf("[Operatore %d]: non sono riuscito a ritirare %d lenzuola pulite. Riprovo fra poco.\n", id, richiesta.qty)
			sleepRandTime(8)
		}
	}

	done <- true
}

func addetto(id int) {
	fmt.Printf("[Addetto %d]: avviato\n", id)

	richiesta := Richiesta{id: id, qty: rand.Intn(MAX_P), ack: make(chan int)}
	var ok bool = true

	for ok {
		ritira_AL <- richiesta
		ris := <-richiesta.ack

		if ris == 1 {
			fmt.Printf("[Addetto %d]: ritirate %d lenzuola sporche.\n", id, richiesta.qty)
			ok = false
		} else {
			fmt.Printf("[Addetto %d]: non sono riuscito a ritirare %d lenzuola sporche. Riprovo fra poco.\n", id, richiesta.qty)
			sleepRandTime(8)
		}
	}

	sleepRandTime(5)
	ok = true

	for ok {
		if richiesta.qty < MAX_P/2 {
			deposita_AL_under <- richiesta
		} else {
			deposita_AL_over <- richiesta
		}

		ris := <-richiesta.ack

		if ris == 1 {
			fmt.Printf("[Addetto %d]: depositate %d lenzuola pulite.\n", id, richiesta.qty)
			ok = false
		} else {
			fmt.Printf("[Addetto %d]: non sono riuscito a depositare %d lenzuola sporche. Riprovo fra poco.\n", id, richiesta.qty)
			sleepRandTime(8)
		}
	}

	done <- true
}

func deposito() {
	fmt.Printf("[Deposito]: avviato\n")

	var num_s int = 70
	var num_p int = 40

	for {
		select {
		case richiesta := <-when((num_p > num_s || (len(deposita_AL_over) == 0 && len(deposita_AL_under) == 0 && len(ritira_AL) == 0)), consegna_OO_under):
			if num_s+richiesta.qty <= MAX_S {
				num_s += richiesta.qty
				richiesta.ack <- 1
				fmt.Printf("[Deposito]: depositate da OO %d sporche. Num_s: %d. Num_p: %d.\n", richiesta.qty, num_s, num_p)
			} else {
				richiesta.ack <- 0
				fmt.Printf("[Deposito]: impossibile depositare da OO %d sporche. Num_s: %d. Num_p: %d.\n", N, num_s, num_p)
			}

		case richiesta := <-when((num_p > num_s || (len(consegna_OO_under) == 0 && len(deposita_AL_over) == 0 && len(deposita_AL_under) == 0 && len(ritira_AL) == 0)), consegna_OO_over):
			if num_s+richiesta.qty <= MAX_S {
				num_s += richiesta.qty
				richiesta.ack <- 1
				fmt.Printf("[Deposito]: depositate da OO %d sporche. Num_s: %d. Num_p: %d.\n", richiesta.qty, num_s, num_p)
			} else {
				richiesta.ack <- 0
				fmt.Printf("[Deposito]: impossibile depositare da OO %d sporche. Num_s: %d. Num_p: %d.\n", N, num_s, num_p)
			}

		case richiesta := <-when((num_p > num_s || (len(ritira_AL) == 0 && len(deposita_AL_over) == 0 && len(deposita_AL_under) == 0)), ritira_OO):
			if num_p >= richiesta.qty {
				num_p -= richiesta.qty
				richiesta.ack <- 1
				fmt.Printf("[Deposito]: ritirate da OO %d pulite. Num_s: %d. Num_p: %d.\n", richiesta.qty, num_s, num_p)
			} else {
				richiesta.ack <- 0
				fmt.Printf("[Deposito]: impossibile ritirare da OO %d pulite. Num_s: %d. Num_p: %d.\n", richiesta.qty, num_s, num_p)
			}

		case richiesta := <-when((num_s > num_p || (len(consegna_OO_over) == 0 && len(consegna_OO_under) == 0 && len(ritira_OO) == 0)), deposita_AL_under):
			if num_p+richiesta.qty <= MAX_P {
				num_p += richiesta.qty
				richiesta.ack <- 1
				fmt.Printf("[Deposito]: depositate da AL %d pulite. Num_s: %d. Num_p: %d.\n", richiesta.qty, num_s, num_p)
			} else {
				richiesta.ack <- 0
				fmt.Printf("[Deposito]: impossibile depositare da AL %d pulite. Num_s: %d. Num_p: %d.\n", richiesta.qty, num_s, num_p)
			}

		case richiesta := <-when((num_s > num_p || (len(deposita_AL_under) == 0 && len(consegna_OO_over) == 0 && len(consegna_OO_under) == 0 && len(ritira_OO) == 0)), deposita_AL_over):
			if num_p+richiesta.qty <= MAX_P {
				num_p += richiesta.qty
				richiesta.ack <- 1
				fmt.Printf("[Deposito]: depositate da AL %d pulite. Num_s: %d. Num_p: %d.\n", richiesta.qty, num_s, num_p)
			} else {
				richiesta.ack <- 0
				fmt.Printf("[Deposito]: impossibile depositare da AL %d pulite. Num_s: %d. Num_p: %d.\n", richiesta.qty, num_s, num_p)
			}

		case richiesta := <-when((num_s > num_p || (len(consegna_OO_over) == 0 && len(consegna_OO_under) == 0 && len(ritira_OO) == 0)), ritira_AL):
			if num_s >= richiesta.qty {
				num_s -= richiesta.qty
				richiesta.ack <- 1
				fmt.Printf("[Deposito]: ritirate da AL %d sporche. Num_s: %d. Num_p: %d.\n", richiesta.qty, num_s, num_p)
			} else {
				richiesta.ack <- 0
				fmt.Printf("[Deposito]: impossibile ritirare da AL %d sporche. Num_s: %d. Num_p: %d.\n", richiesta.qty, num_s, num_p)
			}

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
