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
const N = 20 // capacità
const CREDITORE = 1
const DEBITORE = -1

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
// invece che fare 6 canali si poteva farne 3 ma array, e gestirli con gli indici
var entrata1 = make(chan Richiesta, MAXBUFF)
var entrata2 = make(chan Richiesta, MAXBUFF)
var cassa_debitore = make(chan Richiesta, MAXBUFF)
var cassa_creditore = make(chan Richiesta, MAXBUFF)
var lascia = make(chan Richiesta)
var uscita1 = make(chan Richiesta)
var uscita2 = make(chan Richiesta)

/////////////////////////////////////////////////////////////////////
// Join Goroutine
/////////////////////////////////////////////////////////////////////
var done = make(chan bool, MAXBUFF)
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

func gioca() int {
	tipo := rand.Intn(1) + 1

	if tipo == 0 {
		return 1
		// creditore
	}

	return -1
	// debitore
}

/////////////////////////////////////////////////////////////////////
// Goroutine
/////////////////////////////////////////////////////////////////////
func Visitatore(id int, tipo int) {
	//time.Sleep(time.Duration(rand.Intn(5) + 1) * time.Second)

	richiesta := Richiesta{id, make(chan int)}

	if tipo == 1 {
		entrata1 <- richiesta
		<-richiesta.ack
		fmt.Printf("[Visitatore %d]: entrato nell'ingresso 1", id)
	} else if tipo == 2 {
		entrata2 <- richiesta
		<-richiesta.ack
		fmt.Printf("[Visitatore %d]: entrato nell'ingresso 2", id)
	}

	// si poteva gestire anche in un'altro modo: simulando più partite e ognuna con una vincita e una perdita, il risultato è il medesimo
	time.Sleep(time.Duration(rand.Intn(5)+1) * time.Second)
	saldo := gioca()

	if saldo > 0 {
		fmt.Printf("[Visitatore %d]: ha giocato ed è CREDITORE", id)
	} else {
		fmt.Printf("[Visitatore %d]: ha giocato ed è DEBITORE", id)
	}

	if tipo == 1 {
		uscita1 <- richiesta
		<-richiesta.ack
		fmt.Printf("[Visitatore %d]: uscito dall'ingresso 1", id)
	} else if tipo == 2 {
		uscita2 <- richiesta
		<-richiesta.ack
		fmt.Printf("[Visitatore %d]: uscito dall'ingresso 2", id)
	}

	if saldo > 0 {
		cassa_creditore <- richiesta
		<-richiesta.ack
		fmt.Printf("[Visitatore %d]: ha riscosso il suo credito", id)
	} else {
		cassa_debitore <- richiesta
		<-richiesta.ack
		fmt.Printf("[Visitatore %d]: ha pagato il suo debito", id)
	}

	lascia <- richiesta
	<-richiesta.ack
	fmt.Printf("[Visitatore %d]: lascia il casino", id)

	done <- true
}

func Sala() {
	var numero_1 int = 0
	var numero_2 int = 0

	for {
		select {
		case richiesta := <-when((numero_1+numero_2 <= N && (numero_1 <= numero_2 || len(entrata2) == 0)), entrata1):
			numero_1++
			richiesta.ack <- 1
			fmt.Printf("[Sala]: entrato visitatore %d nell'entrata 1", richiesta.id)
		case richiesta := <-when((numero_1+numero_2 <= N && (numero_2 < numero_1 || len(entrata1) == 0)), entrata2):
			numero_2++
			richiesta.ack <- 1
			fmt.Printf("[Sala]: entrato visitatore %d nell'entrata 2", richiesta.id)
		case richiesta := <-uscita1:
			numero_1--
			richiesta.ack <- 1
			fmt.Printf("[Sala]: uscito visitatore %d dall'entrata 1", richiesta.id)
		case richiesta := <-uscita2:
			numero_2--
			richiesta.ack <- 1
			fmt.Printf("[Sala]: uscito visitatore %d dall'entrata 1", richiesta.id)
		case <-termina:
			fmt.Println("[Sala]: Fine")
			done <- true
			return
		}
	}
}

func Cassa() {
	var libero bool = false

	for {
		select {
		case richiesta := <-when(libero, cassa_debitore):
			libero = false
			richiesta.ack <- 1
			fmt.Printf("[Cassa]: visitatore %d ha pagato il debito", richiesta.id)
		case richiesta := <-when(libero && len(cassa_debitore) == 0, cassa_creditore):
			libero = false
			richiesta.ack <- 1
			fmt.Printf("[Cassa]: visitatore %d ha riscosso il credito", richiesta.id)
		case richiesta := <-lascia:
			richiesta.ack <- 1
			libero = true
			fmt.Printf("[Cassa]: visitatore %d ha lasciato il casino", richiesta.id)
		case <-termina:
			fmt.Println("[Cassa]: Fine")
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

	for i := 0; i <= N; i++ {
		go Visitatore(i, rand.Intn(1)+1)
	}

	go Sala()
	go Cassa()

	for i := 0; i <= N; i++ {
		<-done
	}

	termina <- true
	<-done
	termina <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
