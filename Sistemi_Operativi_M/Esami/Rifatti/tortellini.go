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
const MAX = 50

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

func whenT(b bool, c chan Ticket) chan Ticket {
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
			fmt.Printf("[Spoglina %d]: termino\n", id)
			done <- true
			return
		}

		fmt.Printf("[Spoglina %d]: tortellini depositati\n", id)

		sleepRandTime(3)
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
	ris := <-ticket.ack

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
	var prenotati int = 0
	var fine bool = false
	var fine_deposito bool = false
	var trovato bool = false
	var ticket = make([]int, MAXBUFF)
	var i int = 0

	for {
		select {
		case richiesta := <-when(!fine && num <= MAX && len(prenota) == 0 && len(ritira) == 0, deposita):
			if num == MAX {
				fine_deposito = true
				richiesta.ack <- -1
			} else {
				num++
				richiesta.ack <- 1
				fmt.Printf("[Gestore]: depositati tortellini. NumPronti: %d, NumPrenotati: %d\n", num, prenotati)
			}
		case richiesta := <-when(!fine, prenota):
			prenotati++
			if prenotati >= MAX {
				richiesta.ack <- -1
				fmt.Printf("[Gestore]: impossibile prenotare altri tortellini. NumPronti: %d, NumPrenotati: %d\n", num, prenotati)
			} else {
				ticket[i] = richiesta.id
				i++
				richiesta.ack <- richiesta.id
				fmt.Printf("[Gestore]: tortellini prenotati. NumPronti: %d, NumPrenotati: %d\n", num, prenotati)
			}
		case richiesta := <-whenT(!fine && prenotati > 0 && num > 0 && len(prenota) == 0, ritira):
			fmt.Printf("[Gestore]: ritira\n")
			var j int = 0
			for j = 0; j < i && !trovato; j++ {
				if ticket[j] == richiesta.ticket {
					trovato = true
				}
			}

			if trovato {
				num--
				richiesta.ack <- 1
				fmt.Printf("[Gestore]: tortellini ritirati. NumPronti: %d, NumPrenotati: %d\n", num, prenotati)
			} else {
				richiesta.ack <- -1
				fmt.Printf("[Gestore]: impossibile ritirare tortellini tortellini. NumPronti: %d, NumPrenotati: %d\n", num, prenotati)
			}

			if fine_deposito && num == 0 {
				fine = true
				fmt.Printf("[Gestore]: finiti i ritiri\n")
			}

			trovato = false
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

	var S int = 10
	var C int = 45

	go gestore()

	for i := 0; i < S; i++ {
		go spoglina(i)
	}

	for i := 0; i < C; i++ {
		go cliente(i)
	}

	// join
	for i := 0; i < S+C; i++ {
		<-done
	}

	termina <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
