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
const MAXC = 10
const C = 0
const M = 1
const CC = 0
const CM = 1
const CF = 2

var tipoTorta = [2]string{"Torta Cioccolato", "Crostata Marmellata"}
var tipoConf = [3]string{"Cioccolato", "Marmellata", "Famiglia"}

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
var deposito_cioccolata = make(chan Richiesta, MAXBUFF)
var deposito_marmellata = make(chan Richiesta, MAXBUFF)
var prelievo_confezione_cioccolata = make(chan Richiesta, MAXBUFF)
var prelievo_confezione_marmellata = make(chan Richiesta, MAXBUFF)
var prelievo_confezione_famiglia = make(chan Richiesta, MAXBUFF)

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
func operatore_p(id int) {
	richiesta := Richiesta{id, make(chan int)}
	fmt.Printf("[Produttore %d]: avviato\n", id)

	for {
		tipo := rand.Intn(2)

		if tipo == C {
			deposito_cioccolata <- richiesta
			<-richiesta.ack
			fmt.Printf("[Produttore %d]: ho depositato una %s\n", id, tipoTorta[tipo])
		} else if tipo == M {
			deposito_marmellata <- richiesta
			<-richiesta.ack
			fmt.Printf("[Produttore %d]: ho depositato una %s\n", id, tipoTorta[tipo])
		}

		sleepRandTime(10)
	}
}

func operatore_c(id int) {
	richiesta := Richiesta{id, make(chan int)}
	var ris int
	fmt.Printf("[Confezionatore %d]: avviato\n", id)

	for {
		tipo := rand.Intn(3)

		if tipo == CC {
			prelievo_confezione_cioccolata <- richiesta
			ris = <-richiesta.ack
		} else if tipo == CM {
			prelievo_confezione_marmellata <- richiesta
			ris = <-richiesta.ack
		} else if tipo == CF {
			prelievo_confezione_marmellata <- richiesta
			ris = <-richiesta.ack
		}

		if ris == 0 {
			fmt.Printf("[Confezionatore %d]: termino\n", id)
			done <- true
			return
		}

		fmt.Printf("[Confezionatore %d]: prelevata confezione %s\n", id, tipoConf[tipo])

		sleepRandTime(10)
	}
}

func gestore() {
	var running bool = true
	var num_c int = 0
	var num_m int = 0
	var cc int = 0
	var cm int = 0
	var cf int = 0
	var cc_max int = 10
	var cm_max int = 10
	var cf_max int = 10

	for {
		select {
		case richiesta := <-when(running && num_c+num_m < MAXC && len(deposito_marmellata) == 0, deposito_cioccolata):
			num_c++
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: depositata torta al cioccolato. Num_C: %d, Num_M: %d, Num_CC: %d, Num_CM: %d, Num_CF: %d\n", num_c, num_m, cc, cm, cf)
		case richiesta := <-when(running && num_c+num_m < MAXC, deposito_marmellata):
			num_m++
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: depositata crostata alla marmellata. Num_C: %d, Num_M: %d, Num_CC: %d, Num_CM: %d, Num_CF: %d\n", num_c, num_m, cc, cm, cf)
		case richiesta := <-when(running && num_c > 0 && cc < cc_max && len(prelievo_confezione_famiglia) == 0 && len(prelievo_confezione_marmellata) == 0, prelievo_confezione_cioccolata):
			num_c--
			cc++
			if cc == cc_max {
				richiesta.ack <- 0
			}
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: prelevata confezoine al cioccolato. Num_C: %d, Num_M: %d, Num_CC: %d, Num_CM: %d, Num_CF: %d\n", num_c, num_m, cc, cm, cf)
		case richiesta := <-when(running && num_m > 0 && cm < cm_max && len(prelievo_confezione_famiglia) == 0, prelievo_confezione_marmellata):
			num_m--
			cm++
			if cm == cm_max {
				richiesta.ack <- 0
			}
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: prelevata confezoine al marmellata. Num_C: %d, Num_M: %d, Num_CC: %d, Num_CM: %d, Num_CF: %d\n", num_c, num_m, cc, cm, cf)
		case richiesta := <-when(running && num_c > 0 && num_m > 0 && cf < cf_max, prelievo_confezione_famiglia):
			num_c--
			num_m--
			cf++
			if cf == cf_max {
				richiesta.ack <- 0
			}
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: prelevata confezoine al cioccolato. Num_C: %d, Num_M: %d, Num_CC: %d, Num_CM: %d, Num_CF: %d\n", num_c, num_m, cc, cm, cf)
		case richiesta := <-when(running, deposito_cioccolata):
			richiesta.ack <- -1
		case richiesta := <-when(running, deposito_marmellata):
			richiesta.ack <- -1
		case <-termina:
			if running {
				fmt.Println("[Gestore]: start termination")
				running = false
			} else {
				fmt.Println("[Gestore]: terminato")
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

	var V1 int = 10
	var V2 int = 15

	go gestore()

	for i := 0; i < V1; i++ {
		go operatore_p(i)
	}

	for i := 0; i < V2; i++ {
		go operatore_c(i)
	}

	// join
	for i := 0; i < V1; i++ {
		<-done
	}

	termina <- true

	for i := 0; i < V2; i++ {
		<-done
	}

	termina <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
