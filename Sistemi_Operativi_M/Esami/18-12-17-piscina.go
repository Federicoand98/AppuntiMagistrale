package main

import (
	"fmt"
	"math/rand"
	"time"
)

/////////////////////////////////////////////////////////////////////
//Constants
/////////////////////////////////////////////////////////////////////
const MAXBUFF = 100
const MAXPROC = 10
const MAX = 5 // capacità
const ORDINARI = 0
const STUDENTI = 1
const ORDINARI_ABB = 2
const STUDENTI_ABB = 3

/////////////////////////////////////////////////////////////////////
//Structures
/////////////////////////////////////////////////////////////////////
type Richiesta struct {
	ID  int
	ack chan int
}

/////////////////////////////////////////////////////////////////////
//Channels
/////////////////////////////////////////////////////////////////////
var richiesta_chiave = make(chan Richiesta, MAXBUFF)
var consegna_chiave = make(chan Richiesta, MAXBUFF)
var richiesta_ingresso [4]chan Richiesta
var richiesta_uscita [4]chan Richiesta

/////////////////////////////////////////////////////////////////////
//GORoutine join
/////////////////////////////////////////////////////////////////////
var done = make(chan bool)
var termina = make(chan bool)

/////////////////////////////////////////////////////////////////////
// Auxiliary functions
/////////////////////////////////////////////////////////////////////
func when(b bool, c chan Richiesta) chan Richiesta {
	if !b {
		return nil
	}
	return c
}

func printTipo(typ int) string {
	switch typ {
	case ORDINARI:
		return "ordinario"
	case STUDENTI:
		return "studente"
	case ORDINARI_ABB:
		return "ordinario con abbonamento"
	case STUDENTI_ABB:
		return "studente con abbonamento"
	default:
		return ""
	}
}

/////////////////////////////////////////////////////////////////////
//GORoutines
/////////////////////////////////////////////////////////////////////
func Utente(id int, tipo int) {
	richiesta := Richiesta{id, make(chan int)}

	richiesta_chiave <- richiesta
	<-richiesta.ack
	fmt.Println("[", printTipo(tipo), " ", id, "]: richiesta chiave.")

	fmt.Println("[", printTipo(tipo), " ", id, "]: richiesto ingresso in piscina.")
	richiesta_ingresso[tipo] <- richiesta
	<-richiesta.ack

	fmt.Println("[", printTipo(tipo), " ", id, "]: nuota.")
	time.Sleep(time.Duration(rand.Intn(5)+1) * time.Second)

	richiesta_uscita[tipo] <- richiesta
	<-richiesta.ack

	fmt.Println("[", printTipo(tipo), " ", id, "]: consegna la chiave.")
	consegna_chiave <- richiesta
	<-richiesta.ack

	fmt.Println("[", printTipo(tipo), " ", id, "]: esce.")
	done <- true
}

func Biglietteria() {
	fmt.Printf("[Biglietteria]: start")

	for {
		select {
		case richiesta := <-richiesta_chiave:
			richiesta.ack <- 1
			fmt.Println("[Biglietteria]: utente ", richiesta.ID, " ha richiesto una chiave.")
		case richiesta := <-consegna_chiave:
			richiesta.ack <- 1
			fmt.Println("[Biglietteria]: utente ", richiesta.ID, " ha consegnato una chiave.")
		case <-termina:
			done <- true
			return
		}
	}
}

func Piscina() {
	fmt.Println("[Piscina]: start")

	var ordinari int = 0
	var studenti int = 0

	for {
		select {
		case richiesta := <-when():

		}
	}

}

/////////////////////////////////////////////////////////////////////
//Test main
/////////////////////////////////////////////////////////////////////
func main() {
	var V1 int
	var V2 int

	fmt.Printf("\nQuanti Thread tipo1 (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &V1)
	fmt.Printf("\nQuanti Thread tipo2 (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &V2)

	//inizializzazione canali per le auto a nord e a sud
	for i := 0; i < V1; i++ {
		ACK_T1[i] = make(chan int, MAXBUFF)
	}
	for i := 0; i < V2; i++ {
		ACK_T2[i] = make(chan int, MAXBUFF)
	}

	rand.Seed(time.Now().Unix())
	go server()

	for i := 0; i < T1; i++ {
		go goroutine(i, 1)
	}
	for i := 0; i < T2; i++ {
		go goroutine(i, 0)
	}

	for i := 0; i < V1+V2; i++ {
		<-done
	}

	termina <- true
	<-done
	fmt.Printf("\nHO FINITO!!! ^_- \n")
}
