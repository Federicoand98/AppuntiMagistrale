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
const TS = 25
const TI = 30
const SUPERIORE = 0
const INFERIORE = 1
const PRIMA_CLASSE = 0
const ECONOMY = 1

var tipoViaggiatore = [2]string{"PrimaClasse", "Economy"}
var numero_sopra int = 0
var numero_sotto int = 0

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
var salita_viaggiatore = make(chan Richiesta, MAXBUFF)
var salita_equipaggio = make(chan Richiesta, MAXBUFF)
var discesa_viaggiatore = make(chan Richiesta, MAXBUFF)
var discesa_equipaggio = make(chan Richiesta, MAXBUFF)

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
func viaggiatore(id int, tipo int) {
	richiesta := Richiesta{id, make(chan int)} // se si usa struttura

	if tipo == ECONOMY {
		fmt.Printf("[Viaggiatore economy %d]: avviato.\n", id)
		numero_sotto++

		salita_viaggiatore <- richiesta
		<-richiesta.ack
		fmt.Printf("[Viaggiatore economy %d]: da economy è salito in prima classe\n", id)

		sleepRandTime(7)

		discesa_viaggiatore <- richiesta
		<-richiesta.ack
		fmt.Printf("[Viaggiatore economy %d]: da prima classe è sceso in economy\n", id)
	} else if tipo == PRIMA_CLASSE {
		fmt.Printf("[Viaggiatore prima classe %d]: avviato.\n", id)
		numero_sopra++

		discesa_viaggiatore <- richiesta
		<-richiesta.ack
		fmt.Printf("[Viaggiatore prima classe %d]: da prima classe è sceso in economy\n", id)

		sleepRandTime(7)

		salita_viaggiatore <- richiesta
		<-richiesta.ack
		fmt.Printf("[Viaggiatore prima classe %d]: da economy è salito in prima classe\n", id)
	}

	done <- true
}

func equipaggio(id int, posizione int) {
	richiesta := Richiesta{id, make(chan int)}

	fmt.Printf("[Equipaggio %d]: avviato in posizione %s\n", id, tipoViaggiatore[posizione])

	if posizione == SUPERIORE {
		numero_sopra++

		for {
			discesa_equipaggio <- richiesta
			ris := <-richiesta.ack

			if ris == -1 {
				fmt.Printf("[Equipaggio %d]: termino\n", id)
				done <- true
				return
			} else {
				fmt.Printf("[Equipaggio %d]: scende da prima classe ad economy\n", id)
			}

			sleepRandTime(10)

			salita_equipaggio <- richiesta
			res := <-richiesta.ack

			if res == -1 {
				fmt.Printf("[Equipaggio %d]: termina\n", id)
				done <- true
				return
			} else {
				fmt.Printf("[Equipaggio %d]: sale da economy a prima classe\n", id)
			}

			sleepRandTime(10)
		}
	} else if posizione == INFERIORE {
		numero_sotto++

		for {
			salita_equipaggio <- richiesta
			ris := <-richiesta.ack

			if ris == -1 {
				fmt.Printf("[Equipaggio %d]: termino\n", id)
				done <- true
				return
			} else {
				fmt.Printf("[Equipaggio %d]: sale da economy a prima classe\n", id)
			}

			sleepRandTime(10)

			discesa_equipaggio <- richiesta
			res := <-richiesta.ack

			if res == -1 {
				fmt.Printf("[Equipaggio %d]: termina\n", id)
				done <- true
				return
			} else {
				fmt.Printf("[Equipaggio %d]: s da prima classe ad economy\n", id)
			}

			sleepRandTime(10)
		}
	}
}

func scala() {
	var running bool = true

	for {
		select {
		case richiesta := <-when(running && numero_sopra < TS && (numero_sopra+numero_sotto < TI+TS) && len(salita_equipaggio) == 0 && ((numero_sotto > numero_sopra) || (numero_sotto <= numero_sopra && len(discesa_viaggiatore) == 0 && len(discesa_equipaggio) == 0)), salita_viaggiatore):
			numero_sopra++
			numero_sotto--
			richiesta.ack <- 1
			fmt.Printf("[Scala]: salito viaggiatore. NumSopra: %d. NumSotto: %d\n", numero_sopra, numero_sotto)
		case richiesta := <-when(running && numero_sotto < TI && (numero_sopra+numero_sotto < TI+TS) && len(discesa_equipaggio) == 0 && ((numero_sopra > numero_sotto) || (numero_sopra <= numero_sotto && len(salita_viaggiatore) == 0 && len(salita_equipaggio) == 0)), discesa_viaggiatore):
			numero_sopra--
			numero_sotto++
			richiesta.ack <- 1
			fmt.Printf("[Scala]: disceso viaggiatore. NumSopra: %d. NumSotto: %d\n", numero_sopra, numero_sotto)
		case richiesta := <-when(running && numero_sopra < TS && (numero_sopra+numero_sotto < TI+TS) && ((numero_sotto > numero_sopra) || (numero_sotto <= numero_sopra && len(discesa_viaggiatore) == 0 && len(discesa_equipaggio) == 0)), salita_equipaggio):
			numero_sopra++
			numero_sotto--
			richiesta.ack <- 1
			fmt.Printf("[Scala]: salito equipaggio. NumSopra: %d. NumSotto: %d\n", numero_sopra, numero_sotto)
		case richiesta := <-when(running && numero_sotto < TI && (numero_sopra+numero_sotto < TI+TS) && ((numero_sopra > numero_sotto) || (numero_sopra <= numero_sotto && len(salita_viaggiatore) == 0 && len(salita_equipaggio) == 0)), discesa_equipaggio):
			numero_sopra--
			numero_sotto++
			richiesta.ack <- 1
			fmt.Printf("[Scala]: disceso equipaggio. NumSopra: %d. NumSotto: %d\n", numero_sopra, numero_sotto)
		case richiesta := <-when(!running, salita_equipaggio):
			richiesta.ack <- -1
		case richiesta := <-when(!running, discesa_equipaggio):
			richiesta.ack <- -1
		case <-termina:
			if running {
				fmt.Println("[Scala]: start termination")
				running = false
			} else {
				fmt.Println("[Scala]: terminato")
				done <- true
				return
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////
// Main
/////////////////////////////////////////////////////////////////////
func main() {
	rand.Seed(time.Now().Unix())

	var NV int = 50
	var NE int = 10

	go scala()

	for i := 0; i < NV; i++ {
		go viaggiatore(i, rand.Intn(2))
	}

	for i := 0; i < NE; i++ {
		go equipaggio(i, rand.Intn(2))
	}

	// join
	for i := 0; i < NV; i++ {
		<-done
	}

	termina <- true

	for i := 0; i < NE; i++ {
		<-done
	}

	termina <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
