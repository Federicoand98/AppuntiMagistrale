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
const TRIENNALE = 0
const TRIENNALE_L = 1
const MAGISTRALE = 2
const MAGISTRALE_L = 3

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
var consegna_documento = make(chan Richiesta, MAXBUFF)
var ritira_documento = make(chan Richiesta, MAXBUFF)
var entra [4]chan Richiesta
var esci [4]chan Richiesta

var tipoStudente = [4]string{"Triennale", "Triennale Laureando", "Magistrale", "Magistrale Laureando"}

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
func studente(id int, tipo int) {
	richiesta := Richiesta{id, make(chan int)} // se si usa struttura

	consegna_documento <- richiesta
	<-richiesta.ack
	fmt.Printf("[Studente %d %s]: documento consegnato\n", id, tipoStudente[tipo])

	sleepRandTime(3)

	entra[tipo] <- richiesta
	<-richiesta.ack
	fmt.Printf("[Studente %d %s]: entrato in biblioteca\n", id, tipoStudente[tipo])

	sleepRandTime(10)

	esci[tipo] <- richiesta
	<-richiesta.ack
	fmt.Printf("[Studente %d %s]: uscito dalla biblioteca\n", id, tipoStudente[tipo])

	sleepRandTime(3)
	ritira_documento <- richiesta
	<-richiesta.ack
	fmt.Printf("[Studente %d %s]: documento ritirato, esco\n", id, tipoStudente[tipo])

	done <- true
}

func portineria() {
	var documenti int = 0

	for {
		select {
		case richiesta := <-consegna_documento:
			documenti++
			richiesta.ack <- 1
			fmt.Printf("[Portineria]: studente %d ha consegnato il documento. NumDocumenti: %d\n", richiesta.id, documenti)
		case richiesta := <-ritira_documento:
			documenti--
			richiesta.ack <- 1
			fmt.Printf("[Portineria]: studente %d ha ritirato il documento. NumDocumenti: %d\n", richiesta.id, documenti)
		case <-termina:
			fmt.Printf("[Portineria]: termino\n")
			done <- true
			return
		}
	}
}

func biblioteca() {
	var num int = 0
	var num_t int = 0
	var num_tl int = 0
	var num_m int = 0
	var num_ml int = 0
	var tot_t int = 0
	var tot_m int = 0

	for {
		select {
		case richiesta := <-when(true, entra[TRIENNALE]):
			num++
			num_t++
			tot_t++
			richiesta.ack <- 1
			fmt.Printf("[Biblioteca]: entrato triennale. Num_T: %d, Num_TL: %d, Num_M: %d, Num_ML: %d, Tot: %d\n", num_t, num_tl, num_m, num_ml, num)
		case richiesta := <-when(true, entra[TRIENNALE_L]):
			num++
			num_tl++
			tot_t++
			richiesta.ack <- 1
			fmt.Printf("[Biblioteca]: entrato triennale laureando. Num_T: %d, Num_TL: %d, Num_M: %d, Num_ML: %d, Tot: %d\n", num_t, num_tl, num_m, num_ml, num)
		case richiesta := <-when(true, entra[MAGISTRALE]):
			num++
			num_m++
			tot_m++
			richiesta.ack <- 1
			fmt.Printf("[Biblioteca]: entrato magistrale. Num_T: %d, Num_TL: %d, Num_M: %d, Num_ML: %d, Tot: %d\n", num_t, num_tl, num_m, num_ml, num)
		case richiesta := <-when(num < MAX && (tot_m < tot_t || (tot_m >= tot_t && len(entra[TRIENNALE]) == 0 && len(entra[TRIENNALE_L]) == 0 && len(entra[MAGISTRALE]) == 0)), entra[MAGISTRALE_L]):
			num++
			num_ml++
			tot_m++
			richiesta.ack <- 1
			fmt.Printf("[Biblioteca]: entrato magistrale laureando. Num_T: %d, Num_TL: %d, Num_M: %d, Num_ML: %d, Tot: %d\n", num_t, num_tl, num_m, num_ml, num)
		case richiesta := <-when(true, esci[TRIENNALE]):
			num--
			num_t--
			tot_t--
			richiesta.ack <- 1
			fmt.Printf("[Biblioteca]: uscito triennale. Num_T: %d, Num_TL: %d, Num_M: %d, Num_ML: %d, Tot: %d\n", num_t, num_tl, num_m, num_ml, num)
		case richiesta := <-when(true, esci[TRIENNALE_L]):
			num--
			num_tl--
			tot_t--
			richiesta.ack <- 1
			fmt.Printf("[Biblioteca]: uscito triennale laureando. Num_T: %d, Num_TL: %d, Num_M: %d, Num_ML: %d, Tot: %d\n", num_t, num_tl, num_m, num_ml, num)
		case richiesta := <-when(true, esci[MAGISTRALE]):
			num--
			num_m--
			tot_m--
			richiesta.ack <- 1
			fmt.Printf("[Biblioteca]: uscito magistrale. Num_T: %d, Num_TL: %d, Num_M: %d, Num_ML: %d, Tot: %d\n", num_t, num_tl, num_m, num_ml, num)
		case richiesta := <-when(true, esci[MAGISTRALE_L]):
			num--
			num_ml--
			tot_m--
			richiesta.ack <- 1
			fmt.Printf("[Biblioteca]: uscito magistrale laureando. Num_T: %d, Num_TL: %d, Num_M: %d, Num_ML: %d, Tot: %d\n", num_t, num_tl, num_m, num_ml, num)
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

	for i := 0; i < V1; i++ {
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
