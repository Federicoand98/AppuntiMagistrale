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
const MAX = 15

/////////////////////////////////////////////////////////////////////
// Strutture Dati
/////////////////////////////////////////////////////////////////////
type Richiesta struct {
	id  int
	ack chan int
}

type Ticket struct {
	id     int
	ticket int
	ack    chan int
}

/////////////////////////////////////////////////////////////////////
// Canali
/////////////////////////////////////////////////////////////////////
var deposita = make(chan Richiesta, MAXBUFF)
var prenota = make(chan Richiesta, MAXBUFF)
var ritira = make(chan Ticket, MAXBUFF)

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
func spoglina(id int) {
	richiesta := Richiesta{id, make(chan int)} // se si usa struttura

	for {
		deposita <- richiesta
		ris := <-richiesta.ack

		if ris == -1 {
			fmt.Printf("[Spoglina]: termino\n")
			done <- true
			return
		}

		fmt.Printf("[Spoglina]: tortellini depositati\n")

		sleepRandTime(5)
	}
}

func cliente(id int) {
	richiesta := Richiesta{id, make(chan int)}

	prenota <- richiesta
	codice := <-richiesta.ack

	if codice == -1 {
		fmt.Printf("[Cliente %d]: impossibile prenotare altri torellini, termino\n", id)
		done <- true
		return
	}

	fmt.Printf("[Cliente %d]: tortellini prenotati, ticket: %d\n", id, codice)

	sleepRandTime(5)

	ticket := Ticket{id, codice, make(chan int)}

	ritira <- ticket
	ris := <-richiesta.ack

	if ris == -1 {
		fmt.Printf("[Cliente %d]: impossibile ritirare, il ticket %d non è valido, termino\n", id, ticket.ticket)
		done <- true
		return
	}

	fmt.Printf("[Cliente %d]: tortellini riritati, termino\n", id)

	done <- true
	return
}

func gestore() {
	var num int = 0
	var fine bool = false
	var ticket = make([]int, MAXBUFF)

	for {
		select {
		case richiesta := <-when(!fine && num < MAX && len(prenota) == 0 && len(ritira) == 0, deposita):
			num++
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: depositati tortellini. Num: %d\n", num)
		case richiesta := <-when(!fine, prenota):

		case richiesta := <-when(!fine && num > 0 && len(prenota) == 0, ritira):

		case richiesta := <-when(fine, deposita):
			richiesta.ack <- -1
			fmt.Printf("[Gestore]: termino\n")
		case <-termina:
			fmt.Println("\n\n[SERVER]: Fine!")
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

	var V1 int
	var V2 int

	fmt.Printf("\nQuanti Thread tipo1 (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &V1)
	fmt.Printf("\nQuanti Thread tipo2 (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &V2)

	//inizializzazione canali se non si usa struttura dati
	for i := 0; i < V1; i++ {
		ACK[i] = make(chan int, MAXBUFF)
	}

	go server()

	for i := 0; i < T1; i++ {
		go goroutine(i, 1)
	}

	// join
	for i := 0; i < V1+V2; i++ {
		<-done
	}

	termina <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
