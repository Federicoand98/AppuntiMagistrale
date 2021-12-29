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
const MAX = 15 // capacità
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
var done chan bool
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
		case richiesta := <-when(ordinari+studenti < MAX && (studenti <= ordinari || len(richiesta_ingresso[ORDINARI]) == 0 && len(richiesta_ingresso[ORDINARI_ABB]) == 0), richiesta_ingresso[STUDENTI_ABB]):
			studenti++
			richiesta.ack <- 1
			fmt.Println("[Piscina]: ingresso studente con abbonamento ", richiesta.ID, ". Num studenti: ", studenti, " Num ordinari: ", ordinari)
		case richiesta := <-when(ordinari+studenti < MAX && ((studenti <= ordinari && len(richiesta_ingresso[STUDENTI_ABB]) == 0) || len(richiesta_ingresso[ORDINARI]) == 0 && len(richiesta_ingresso[ORDINARI_ABB]) == 0 && len(richiesta_ingresso[STUDENTI_ABB]) == 0), richiesta_ingresso[STUDENTI]):
			studenti++
			richiesta.ack <- 1
			fmt.Println("[Piscina]: ingresso studente ", richiesta.ID, ". Num studenti: ", studenti, " Num ordinari: ", ordinari)
		case richiesta := <-when(ordinari+studenti < MAX && (ordinari < studenti || len(richiesta_ingresso[STUDENTI_ABB]) == 0 && len(richiesta_ingresso[STUDENTI]) == 0), richiesta_ingresso[ORDINARI_ABB]):
			ordinari++
			richiesta.ack <- 1
			fmt.Println("[Piscina]: ingresso ordinario con abbonamento ", richiesta.ID, ". Num studenti: ", studenti, " Num ordinari: ", ordinari)
		case richiesta := <-when(ordinari+studenti < MAX && ((ordinari < studenti && len(richiesta_ingresso[ORDINARI_ABB]) == 0) || len(richiesta_ingresso[STUDENTI]) == 0 && len(richiesta_ingresso[STUDENTI_ABB]) == 0 && len(richiesta_ingresso[ORDINARI_ABB]) == 0), richiesta_ingresso[ORDINARI]):
			ordinari++
			richiesta.ack <- 1
			fmt.Println("[Piscina]: ingresso ordinario ", richiesta.ID, ". Num studenti: ", studenti, " Num ordinari: ", ordinari)
		case richiesta := <-richiesta_uscita[STUDENTI_ABB]:
			studenti--
			richiesta.ack <- 1
			fmt.Println("[Piscina]: uscita studente con abbonamento ", richiesta.ID, ". Num studenti: ", studenti, " Num ordinari: ", ordinari)
		case richiesta := <-richiesta_uscita[STUDENTI]:
			studenti--
			richiesta.ack <- 1
			fmt.Println("[Piscina]: uscita studente ", richiesta.ID, ". Num studenti: ", studenti, " Num ordinari: ", ordinari)
		case richiesta := <-richiesta_uscita[ORDINARI_ABB]:
			ordinari--
			richiesta.ack <- 1
			fmt.Println("[Piscina]: uscita ordinario con abbonamento ", richiesta.ID, ". Num studenti: ", studenti, " Num ordinari: ", ordinari)
		case richiesta := <-richiesta_uscita[ORDINARI]:
			ordinari--
			richiesta.ack <- 1
			fmt.Println("[Piscina]: uscita ordinario ", richiesta.ID, ". Num studenti: ", studenti, " Num ordinari: ", ordinari)
		case <-termina:
			done <- true
			return
		}
	}
}

/////////////////////////////////////////////////////////////////////
//Test main
/////////////////////////////////////////////////////////////////////
func main() {
	rand.Seed(time.Now().UTC().UnixNano())

	done = make(chan bool, MAX)

	for i := 0; i < 4; i++ {
		richiesta_ingresso[i] = make(chan Richiesta, MAXBUFF)
		richiesta_uscita[i] = make(chan Richiesta, MAXBUFF)
	}

	for i := 0; i < 4; i++ {
		go Utente(i, ORDINARI)
	}

	for i := 4; i < 8; i++ {
		go Utente(i, STUDENTI)
	}

	for i := 8; i < 12; i++ {
		go Utente(i, ORDINARI_ABB)
	}

	for i := 12; i < 16; i++ {
		go Utente(i, STUDENTI_ABB)
	}

	go Biglietteria()
	go Piscina()

	for i := 0; i < 16; i++ {
		<-done
	}

	fmt.Println("terminate")
	termina <- true
	<-done
	termina <- true
	<-done
	fmt.Println("FINITO")
}
