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
const VISITATORE_M = 0
const VISITATORE_S = 1
const VISITATORE_MS = 2
const OPERATORE_M = 0
const OPERATORE_S = 1
const NM = 30
const NS = 20

var tipoUtente = [3]string{"Museo", "Sala Mostre", "Museo+Sala Mostre"}

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
var entrata_visitatori [3]chan Richiesta
var uscita_visitatori [3]chan Richiesta
var entrata_operatori [2]chan Richiesta
var uscita_operatori [2]chan Richiesta

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
func visitatore(id int, tipo int) {
	richiesta := Richiesta{id, make(chan int)}

	entrata_visitatori[tipo] <- richiesta
	<-richiesta.ack

	fmt.Printf("[Visitatore %s %d]: entrato.\n", tipoUtente[tipo], id)

	sleepRandTime(4)

	uscita_visitatori[tipo] <- richiesta
	<-richiesta.ack

	fmt.Printf("[Visitatore %s %d]: uscito.\n", tipoUtente[tipo], id)

	done <- true
}

func operatore(id int, tipo int) {
	richiesta := Richiesta{id, make(chan int)}

	entrata_operatori[tipo] <- richiesta
	<-richiesta.ack

	fmt.Printf("[Operatore %s %d]: entrato.\n", tipoUtente[tipo], id)

	for i := 0; i < 7; i++ {
		sleepRandTime(5)

		uscita_operatori[tipo] <- richiesta
		<-richiesta.ack
		fmt.Printf("[Operatore %s %d]: uscito.\n", tipoUtente[tipo], id)

		sleepRandTime(3)

		entrata_operatori[tipo] <- richiesta
		<-richiesta.ack

		fmt.Printf("[Operatore %s %d]: entrato.\n", tipoUtente[tipo], id)
	}

	done <- true
}

func server() {
	var op_m int = 0
	var op_s int = 0
	var vis_m int = 0
	var vis_s int = 0

	for {
		select {
		case richiesta := <-when((vis_m < NM), entrata_visitatori[VISITATORE_M]):
			vis_m++
			richiesta.ack <- 1

		case richiesta := <-when((vis_s < NS && len(entrata_visitatori[VISITATORE_M]) == 0), entrata_visitatori[VISITATORE_S]):
			vis_s++
			richiesta.ack <- 1

		case richiesta := <-when((vis_m < NM && vis_s < NS && len(entrata_visitatori[VISITATORE_M]) == 0 && len(entrata_visitatori[VISITATORE_S]) == 0), entrata_visitatori[VISITATORE_MS]):
			vis_m++
			vis_s++
			richiesta.ack <- 1

		case richiesta := <-entrata_operatori[OPERATORE_M]:
			op_m++
			richiesta.ack <- 1

		case richiesta := <-entrata_operatori[OPERATORE_S]:
			op_s++
			richiesta.ack <- 1

		case richiesta := <-uscita_visitatori[VISITATORE_M]:
			vis_m--
			richiesta.ack <- 1

		case richiesta := <-uscita_visitatori[VISITATORE_S]:
			vis_s--
			richiesta.ack <- 1

		case richiesta := <-uscita_visitatori[VISITATORE_MS]:
			vis_m--
			vis_s--
			richiesta.ack <- 1

		case richiesta := <-when((op_m > 0 && vis_m > 0) || (len(entrata_visitatori[VISITATORE_M]) == 0), uscita_operatori[OPERATORE_M]):
			op_m--
			richiesta.ack <- 1

		case richiesta := <-when((op_s > 0 && vis_s > 0) || (len(entrata_visitatori[VISITATORE_S]) == 0), uscita_operatori[OPERATORE_S]):
			op_s--
			richiesta.ack <- 1

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
