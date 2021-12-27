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
const N = 5 // sportelli disponibili
const OVER = 0
const UNDER = 1

/////////////////////////////////////////////////////////////////////
//Structures
/////////////////////////////////////////////////////////////////////
type Richiesta struct {
	id  int
	ack chan int
}

/////////////////////////////////////////////////////////////////////
//Channels
/////////////////////////////////////////////////////////////////////
var richiesta_sportello_entra [2]chan Richiesta
var richiesta_sportello_esce [2]chan Richiesta
var consegna_sportello_entra [2]chan Richiesta
var consegna_sportello_esce [2]chan Richiesta
var entrata_caveau [2]chan Richiesta
var uscita_caveau [2]chan Richiesta

/////////////////////////////////////////////////////////////////////
//GORoutine join
/////////////////////////////////////////////////////////////////////
var done = make(chan bool)
var termina_sportelli = make(chan bool)
var termina_caveau = make(chan bool)

/////////////////////////////////////////////////////////////////////
// Auxiliary functions
/////////////////////////////////////////////////////////////////////
func when(b bool, c chan Richiesta) chan Richiesta {
	if !b {
		return nil
	}
	return c
}

func printTipo(tipo int) string {
	switch tipo {
	case OVER:
		return "Over 70"
	case UNDER:
		return "Under 70"
	default:
		return ""
	}
}

/////////////////////////////////////////////////////////////////////
//GORoutines
/////////////////////////////////////////////////////////////////////
func client(id int, tipo int) {
	richiesta := Richiesta{id, make(chan int)}

	richiesta_sportello_entra[tipo] <- richiesta
	<-richiesta.ack
	fmt.Println(printTipo(tipo), " ID: ", id, " richiesta chiave.")

	richiesta_sportello_esce[tipo] <- richiesta
	<-richiesta.ack
	fmt.Println(printTipo(tipo), " ID: ", id, " mi metto in coda per il caveau.")

	entrata_caveau[tipo] <- richiesta
	<-richiesta.ack

	fmt.Println(printTipo(tipo), " ID: ", id, " sono dentro il caveau.")
	time.Sleep(time.Duration(rand.Intn(5)) * time.Second)

	uscita_caveau[tipo] <- richiesta
	<-richiesta.ack
	fmt.Println(printTipo(tipo), " ID: ", id, " sono uscito dal caveau, torno a consegnare la chiave.")

	consegna_sportello_entra[tipo] <- richiesta
	<-richiesta.ack
	fmt.Println(printTipo(tipo), " ID: ", id, " chiave restituita.")

	consegna_sportello_esce[tipo] <- richiesta
	<-richiesta.ack

	done <- true
}

func sportelli() {
	fmt.Printf("Sportelli: start.")

	var coda int = 0

	for {
		select {
		case richiesta := <-when((coda < N), richiesta_sportello_entra[OVER]):
			coda++
			fmt.Printf("[SPORTELLO]: over %d richiesta chiave", richiesta.id)
			richiesta.ack <- 1
		case richiesta := <-when(coda < N && (len(richiesta_sportello_entra[OVER]) == 0), richiesta_sportello_entra[UNDER]):
			coda++
			fmt.Printf("[SPORTELLO]: under %d richiesta chiave", richiesta.id)
			richiesta.ack <- 1
		case richiesta := <-when((coda < N), consegna_sportello_entra[OVER]):
			coda++
			fmt.Printf("[SPORTELLO]: over %d consegnata chiave", richiesta.id)
			richiesta.ack <- 1
		case richiesta := <-when(coda < N && (len(consegna_sportello_entra[OVER]) == 0), consegna_sportello_entra[UNDER]):
			coda++
			fmt.Printf("[SPORTELLO]: under %d consegnata chiave", richiesta.id)
			richiesta.ack <- 1
		case richiesta := <-richiesta_sportello_esce[OVER]:
			coda--
			fmt.Printf("[SPORTELLO]: over %d esce dallo sportello e va al caveau", richiesta.id)
			richiesta.ack <- 1
		case richiesta := <-richiesta_sportello_esce[UNDER]:
			coda--
			fmt.Printf("[SPORTELLO]: under %d esce dallo sportello e va al caveau", richiesta.id)
			richiesta.ack <- 1
		case richiesta := <-consegna_sportello_esce[OVER]:
			coda--
			fmt.Printf("[SPORTELLO]: over %d esce dallo sportello e va via", richiesta.id)
			richiesta.ack <- 1
		case richiesta := <-consegna_sportello_esce[UNDER]:
			coda--
			fmt.Printf("[SPORTELLO]: under %d esce dallo sportello e va via", richiesta.id)
			richiesta.ack <- 1
		case <-termina_sportelli:
			fmt.Printf("[SPORTELLO]: finito")
			done <- true
			return
		}
	}
}

func caveau() {
	fmt.Printf("Caveau: start.")

	var caveau_libero bool = true
	var storico_over int = 0
	var storico_under int = 0

	for {
		select {
		case richiesta := <-when((caveau_libero && (storico_over <= storico_under) || (storico_over > storico_under && len(entrata_caveau[UNDER]) == 0)), entrata_caveau[OVER]):
			storico_over++
			caveau_libero = false
			fmt.Printf("[CAVEAU]: è appena entrato over %d", richiesta.id)
			richiesta.ack <- 1
		case richiesta := <-when((caveau_libero && (storico_under <= storico_over) || (storico_under > storico_over && len(entrata_caveau[OVER]) == 0)), entrata_caveau[UNDER]):
			storico_under++
			caveau_libero = false
			fmt.Printf("[CAVEAU]: è appena entrato under %d", richiesta.id)
			richiesta.ack <- 1
		case richiesta := <-uscita_caveau[OVER]:
			caveau_libero = true
			fmt.Printf("[CAVEAU]: è appena uscito over %d", richiesta.id)
			richiesta.ack <- 1
		case richiesta := <-uscita_caveau[UNDER]:
			caveau_libero = true
			fmt.Printf("[CAVEAU]: è appena uscito under %d", richiesta.id)
			richiesta.ack <- 1
		case <-termina_caveau:
			fmt.Printf("[CAVEAU]: finito")
			done <- true
			return
		}
	}
}

/////////////////////////////////////////////////////////////////////
//Test main
/////////////////////////////////////////////////////////////////////
func main() {
	rand.Seed(time.Now().Unix())

	for i := 0; i < 2; i++ {
		richiesta_sportello_entra[i] = make(chan Richiesta, MAXBUFF)
		richiesta_sportello_esce[i] = make(chan Richiesta, MAXBUFF)
		consegna_sportello_entra[i] = make(chan Richiesta, MAXBUFF)
		consegna_sportello_esce[i] = make(chan Richiesta, MAXBUFF)
		entrata_caveau[i] = make(chan Richiesta, MAXBUFF)
		uscita_caveau[i] = make(chan Richiesta, MAXBUFF)
	}

	for i := 0; i < 20; i++ {
		go client(i, rand.Intn(2))
	}

	go sportelli()
	go caveau()

	for i := 0; i < 20; i++ {
		<-done
	}

	termina_caveau <- true
	<-done
	termina_sportelli <- true
	<-done

	fmt.Printf("Temino")

	fmt.Printf("\nFine.\n")
}
