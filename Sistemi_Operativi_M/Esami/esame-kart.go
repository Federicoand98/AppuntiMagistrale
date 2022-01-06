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
const MA = 0
const MI = 1
const C1 = 0
const C2 = 1
const N1 = 10
const N2 = 12

var tipoUtente = [2]string{"Maggiorenne", "Minorenne"}
var tipoKart = [2]string{"125cc", "50cc"}

/////////////////////////////////////////////////////////////////////
// Strutture Dati
/////////////////////////////////////////////////////////////////////
type Richiesta struct {
	id   int
	kart int
	ack  chan int
}

/////////////////////////////////////////////////////////////////////
// Canali
/////////////////////////////////////////////////////////////////////
var richiesta_kart_ma = make(chan Richiesta, MAXBUFF)
var richiesta_kart_mi = make(chan Richiesta, MAXBUFF)
var consegna_kart_ma = make(chan Richiesta, MAXBUFF)
var consegna_kart_mi = make(chan Richiesta, MAXBUFF)
var ingresso_pista_c1 = make(chan Richiesta, MAXBUFF)
var ingresso_pista_c2 = make(chan Richiesta, MAXBUFF)
var uscita_pista_c1 = make(chan Richiesta, MAXBUFF)
var uscita_pista_c2 = make(chan Richiesta, MAXBUFF)

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
func cliente(id int, tipo int) {
	richiesta := Richiesta{id, 999, make(chan int)} // se si usa struttura
	var ris int
	fmt.Printf("[Cliente %d %s]: avviato.\n", id, tipoUtente[tipo])

	if tipo == MA {
		richiesta_kart_ma <- richiesta
		ris = <-richiesta.ack
		richiesta.kart = ris
		fmt.Printf("[Cliente %d %s]: ottenuto kart %s\n", id, tipoUtente[tipo], tipoKart[ris])

		sleepRandTime(3)

		if ris == C1 {
			ingresso_pista_c1 <- richiesta
			<-richiesta.ack
			fmt.Printf("[Cliente %d %s]: sono entrtato in pista con kart %s\n", id, tipoUtente[tipo], tipoKart[ris])

			sleepRandTime(10)

			uscita_pista_c1 <- richiesta
			<-richiesta.ack
			fmt.Printf("[Cliente %d %s]: sono uscito dalla pista con kart %s\n", id, tipoUtente[tipo], tipoKart[ris])
		} else if ris == C2 {
			ingresso_pista_c2 <- richiesta
			<-richiesta.ack
			fmt.Printf("[Cliente %d %s]: sono entrtato in pista con kart %s\n", id, tipoUtente[tipo], tipoKart[ris])

			sleepRandTime(10)

			uscita_pista_c2 <- richiesta
			<-richiesta.ack
			fmt.Printf("[Cliente %d %s]: sono uscito dalla pista con kart %s\n", id, tipoUtente[tipo], tipoKart[ris])
		}

		sleepRandTime(3)

		consegna_kart_ma <- richiesta
		<-richiesta.ack
		fmt.Printf("[Cliente %d %s]: consegnato kart %s\n", id, tipoUtente[tipo], tipoKart[ris])
	} else if tipo == MI {
		richiesta_kart_mi <- richiesta
		ris = <-richiesta.ack
		richiesta.kart = ris
		fmt.Printf("[Cliente %d %s]: ottenuto kart %s\n", id, tipoUtente[tipo], tipoKart[ris])

		sleepRandTime(3)

		ingresso_pista_c2 <- richiesta
		<-richiesta.ack
		fmt.Printf("[Cliente %d %s]: sono entrtato in pista con kart %s\n", id, tipoUtente[tipo], tipoKart[ris])

		sleepRandTime(10)

		uscita_pista_c2 <- richiesta
		<-richiesta.ack
		fmt.Printf("[Cliente %d %s]: sono uscito dalla pista con kart %s\n", id, tipoUtente[tipo], tipoKart[ris])

		sleepRandTime(3)

		consegna_kart_mi <- richiesta
		<-richiesta.ack
		fmt.Printf("[Cliente %d %s]: consegnato kart %s\n", id, tipoUtente[tipo], tipoKart[ris])
	}

	done <- true
}

func gestore() {
	var disp_c1 int = N1
	var disp_c2 int = N2
	var c1_in int = 0
	var c2_in int = 0
	var occupata_c1 bool = false
	var occupata_c2 bool = false

	for {
		select {
		case richiesta := <-when(disp_c2 > 0 && len(richiesta_kart_ma) == 0, richiesta_kart_mi):
			disp_c2--
			richiesta.ack <- C2
			fmt.Printf("[Gestore]: utente %d richiesto kart %s. DisponibiliC1: %d, DisponibiliC2: %d\n", richiesta.id, tipoKart[C2], disp_c1, disp_c2)
		case richiesta := <-when(disp_c1 > 0 || disp_c2 > 0, richiesta_kart_ma):
			if disp_c1 > 0 {
				disp_c1--
				richiesta.ack <- C1
				fmt.Printf("[Gestore]: utente %d richiesto kart %s. DisponibiliC1: %d, DisponibiliC2: %d\n", richiesta.id, tipoKart[C2], disp_c1, disp_c2)
			} else {
				disp_c2--
				richiesta.ack <- C2
				fmt.Printf("[Gestore]: utente %d richiesto kart %s. DisponibiliC1: %d, DisponibiliC2: %d\n", richiesta.id, tipoKart[C2], disp_c1, disp_c2)
			}
		case richiesta := <-when(true, consegna_kart_mi):
			disp_c2++
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: utente %d consegnato kart %s. DisponibiliC1: %d, DisponibiliC2: %d\n", richiesta.id, tipoKart[C2], disp_c1, disp_c2)
		case richiesta := <-when(true, consegna_kart_ma):
			if richiesta.kart == C1 {
				disp_c1++
				richiesta.ack <- 1
				fmt.Printf("[Gestore]: utente %d consegnato kart %s. DisponibiliC1: %d, DisponibiliC2: %d\n", richiesta.id, tipoKart[C2], disp_c1, disp_c2)
			} else if richiesta.kart == C2 {
				disp_c2++
				richiesta.ack <- 1
				fmt.Printf("[Gestore]: utente %d consegnato kart %s. DisponibiliC1: %d, DisponibiliC2: %d\n", richiesta.id, tipoKart[C2], disp_c1, disp_c2)
			}
		case richiesta := <-when(!occupata_c2, ingresso_pista_c1):
			occupata_c1 = true
			c1_in++
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: utente %d entrato in pista con kart %s. C1Dentro: %d, C2Dentro: %d\n", richiesta.id, tipoKart[C2], c1_in, c2_in)
		case richiesta := <-when(!occupata_c1 && len(ingresso_pista_c1) == 0, ingresso_pista_c2):
			occupata_c2 = true
			c2_in++
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: utente %d entrato in pista con kart %s. C1Dentro: %d, C2Dentro: %d\n", richiesta.id, tipoKart[C2], c1_in, c2_in)
		case richiesta := <-when(true, uscita_pista_c1):
			c1_in--
			if c1_in == 0 {
				occupata_c1 = false
			}
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: utente %d uscito dalla pista con kart %s. C1Dentro: %d, C2Dentro: %d\n", richiesta.id, tipoKart[C2], c1_in, c2_in)
		case richiesta := <-when(true, uscita_pista_c2):
			c2_in--
			if c2_in == 0 {
				occupata_c2 = false
			}
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: utente %d uscito dalla pista con kart %s. C1Dentro: %d, C2Dentro: %d\n", richiesta.id, tipoKart[C2], c1_in, c2_in)
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

	var V1 int = 40

	go gestore()

	for i := 0; i < V1; i++ {
		go cliente(i, rand.Intn(2))
	}

	// join
	for i := 0; i < V1; i++ {
		<-done
	}

	termina <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
