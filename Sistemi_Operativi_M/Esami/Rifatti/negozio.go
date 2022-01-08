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
const MN = 3
const ABITUALI = 0
const OCCASIONALI = 1
const N_COMMESSI = 8

var tipoCliente = [2]string{"Abituale", "Occasionale"}

/////////////////////////////////////////////////////////////////////
// Strutture Dati
/////////////////////////////////////////////////////////////////////
type Richiesta struct {
	id  int
	ack chan int
}

type Commesso struct {
	id           int
	fuori        bool
	vuole_uscire bool
	num_clienti  int
	clienti      [3]int
}

/////////////////////////////////////////////////////////////////////
// Canali
/////////////////////////////////////////////////////////////////////
var entra_cliente [2]chan Richiesta
var esci_cliente [2]chan Richiesta
var entra_commesso = make(chan Richiesta, MAXBUFF)
var esci_commesso = make(chan Richiesta, MAXBUFF)
var deposita_mascherine = make(chan Richiesta, MAXBUFF)

/////////////////////////////////////////////////////////////////////
// Join Goroutine
/////////////////////////////////////////////////////////////////////
var done = make(chan bool)
var termina_negozio = make(chan bool)
var termina_fornitore = make(chan bool)

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
func fornitore() {
	richiesta := Richiesta{0, make(chan int)}

	deposita_mascherine <- richiesta
	<-richiesta.ack

	fmt.Printf("[Fornitore]: depositate %d mascherine\n", MN)

	select {
	case <-termina_fornitore:
		fmt.Printf("[Fornitore]: termino\n")
		done <- true
		return
	default:
		sleepRandTime(3)
	}
}

func cliente(id int, tipo int) {
	richiesta := Richiesta{id, make(chan int)}

	entra_cliente[tipo] <- richiesta
	<-richiesta.ack
	fmt.Printf("[Cliente %d %s]: entrato nel negozio\n", id, tipoCliente[tipo])

	sleepRandTime(5)

	esci_cliente[tipo] <- richiesta
	<-richiesta.ack
	fmt.Printf("[Cliente %d %s]: uscito dal negozio\n", id, tipoCliente[tipo])

	done <- true
}

func commesso(id int) {
	richiesta := Richiesta{id, make(chan int)}
	var ris int

	for {
		sleepRandTime(3)

		entra_commesso <- richiesta
		ris = <-richiesta.ack

		if ris == -1 {
			fmt.Printf("[Commesso %d]: termino\n")
			done <- true
			return
		}

		fmt.Printf("[Commesso %d]: entrato nel negozio\n", id)

		sleepRandTime(5)

		esci_commesso <- richiesta
		ris = <-richiesta.ack

		if ris == -1 {
			fmt.Printf("[Commesso %d]: termino\n", id)
			done <- true
			return
		}

		fmt.Printf("[Commesso %d]: uscito dal negozio\n", id)
	}
}

func server() {
	var running bool = true
	var commessi [N_COMMESSI]Commesso
	var num_commessi int = 0
	var num_clienti int = 0
	var commessi_liberi int = 0

	for i := 0; i < N_COMMESSI; i++ {
		commessi[i].id = 0
		commessi[i].fuori = false
		commessi[i].vuole_uscire = false
		for j := 0; j < 3; j++ {
			commessi[i].clienti[j] = 0
		}
	}

	for {
		select {
		case richiesta := <-when(running, entra_cliente[ABITUALI]):

		case richiesta := <-when(running, entra_cliente[OCCASIONALI]):

		case richiesta := <-when(running, esci_cliente[ABITUALI]):

		case richiesta := <-when(running, esci_cliente[OCCASIONALI]):

		case richiesta := <-when(running, entra_commesso):
			num_commessi++
			commessi_liberi++
			commessi[richiesta.id].fuori = false
			commessi[richiesta.id].vuole_uscire = false
			commessi[richiesta.id].num_clienti = 0
			for i := 0; i < 3; i++ {
				commessi[richiesta.id].clienti[i] = -1
			}
			richiesta.ack <- 1
			fmt.Printf("[Negozio]: entrato commesso. NumClienti: %d, NumCommessi: %d, NumCommessiLiberi: %d\n", num_clienti, num_commessi, commessi_liberi)
		case richiesta := <-when(running, esci_commesso):
			if commessi[richiesta.id].num_clienti == 0 {
				commessi[richiesta.id].fuori = true
				commessi[richiesta.id].vuole_uscire = false
				num_commessi--
				commessi_liberi--
				richiesta.ack <- 1
				fmt.Printf("[Negozio]: commesso %d uscito. NumClienti: %d, NumCommessi: %d, NumCommessiLiberi: %d\n", richiesta.id, num_clienti, num_commessi, commessi_liberi)
			} else {

			}
		case richiesta := <-when(!running, entra_commesso):

		case richiesta := <-when(!running, esci_commesso):

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
