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
const VP = 0
const VM = 1
const ROSSA = 0
const ARANCIONE = 1
const GIALLA = 2
const MAX_P = 200
const MAX_M = 150
const NL = 30
const Q = 15
const TOT_P = 200
const TOT_M = 200

var tipoVaccino = [2]string{"Pfizer-BionTech", "Moderna"}
var tipoZona = [3]string{"Rossa", "Arancione", "Gialla"}
var tipoRegione = [20]string{"Valle D'aosta", "Piemonte", "Lombardia", "Veneto", "Friuli V.G.", "Trentino Alto Adige",
	"Liguria", "Toscana", "Emilia Romagna", "Marche", "Umbria", "Lazio", "Abruzzo", "Molise", "Campania", "Basilicata",
	"Puglia", "Calabria", "Sicilia", "Sardegna"}

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
var prenota [2]chan Richiesta
var deposita [2]chan Richiesta
var prelievo [3]chan Richiesta

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
func produttore(id int, tipo int) {
	richiesta := Richiesta{id, make(chan int)} // se si usa struttura
	var ris int = 0
	fmt.Printf("[Casa %s]: avviata\n", tipoVaccino[tipo])

	for {
		prenota[tipo] <- richiesta
		ris = <-richiesta.ack

		if ris == -1 {
			fmt.Printf("[Casa %s]: termino\n", tipoVaccino[tipo])
			done <- true
			return
		}

		fmt.Printf("[Casa %s]: prenotata consegna di %d vaccini\n", tipoVaccino[tipo], NL)

		sleepRandTime(2)

		deposita[tipo] <- richiesta
		ris = <-richiesta.ack

		if ris == -1 {
			fmt.Printf("[Casa %s]: termino\n", tipoVaccino[tipo])
			done <- true
			return
		}

		fmt.Printf("[Casa %s]: consegnate %d dosi di vaccino\n", tipoVaccino[tipo], NL)

		sleepRandTime(5)
	}
}

func regione(tipo int) {
	richiesta := Richiesta{tipo, make(chan int)} // se si usa struttura

	fmt.Printf("[Regione %d]: avviata\n", tipoRegione[tipo])

	for {
		zona := rand.Intn(3)

		fmt.Printf("[Regione %s]: è in zona %s\n", tipoRegione[tipo], tipoZona[zona])

		prelievo[zona] <- richiesta
		ris := <-richiesta.ack

		if ris == -1 {
			fmt.Printf("[Region %s]: termino\n", tipoRegione[tipo])
			done <- true
			return
		}

		fmt.Printf("[Regione %s]: in zona %s prelevate %d dosi\n", tipoRegione[tipo], tipoZona[zona], Q)

		sleepRandTime(5)
	}

	done <- true
}

func deposito() {
	var running bool = true
	var num_vp int = 0
	var num_vm int = 0
	var prenotati_vp int = 0
	var prenotati_vm int = 0
	var prel int = 0
	var tot_vp int = 0
	var tot_vm int = 0

	for {
		select {
		case richiesta := <-when(running && num_vp+prenotati_vp+NL <= MAX_P, prenota[VP]):
			prenotati_vp += NL
			richiesta.ack <- 1
			fmt.Printf("[Deposito]: prenotato lotto VP. PrenotatiVP: %d, PrenotatiVM: %d, NumVP: %d, NumVM: %d\n", prenotati_vp, prenotati_vm, num_vp, num_vm)
		case richiesta := <-when(running && num_vm+prenotati_vm+NL <= MAX_M, prenota[VM]):
			prenotati_vm += NL
			richiesta.ack <- 1
			fmt.Printf("[Deposito]: prenotato lotto VM. PrenotatiVP: %d, PrenotatiVM: %d, NumVP: %d, NumVM: %d\n", prenotati_vp, prenotati_vm, num_vp, num_vm)
		case richiesta := <-when(running, deposita[VP]):
			num_vp += NL
			prenotati_vp -= NL
			richiesta.ack <- 1
			fmt.Printf("[Deposito]: depositato lotto VP. PrenotatiVP: %d, PrenotatiVM: %d, NumVP: %d, NumVM: %d\n", prenotati_vp, prenotati_vm, num_vp, num_vm)
		case richiesta := <-when(running, deposita[VM]):
			num_vm += NL
			prenotati_vm -= NL
			richiesta.ack <- 1
			fmt.Printf("[Deposito]: depositato lotto VM. PrenotatiVP: %d, PrenotatiVM: %d, NumVP: %d, NumVM: %d\n", prenotati_vp, prenotati_vm, num_vp, num_vm)
		case richiesta := <-when(running && num_vp+num_vm >= Q, prelievo[ROSSA]):
			if num_vp >= Q {
				num_vp -= Q
				tot_vp += Q
				prel = Q
			} else {
				prel = num_vp
				num_vp = 0
				num_vm -= (Q - prel)
				tot_vp += prel
				tot_vm += (Q - prel)
			}
			richiesta.ack <- 1

			if tot_vm == TOT_P && tot_vp == TOT_P {
				running = false
			}

			fmt.Printf("[Deposito]: regione rossa prelevato %d vaccini VP e %d vaccini VM \n", prel, (Q - prel))
		case richiesta := <-when(running && num_vm+num_vp >= Q && len(prelievo[ROSSA]) == 0, prelievo[ARANCIONE]):
			if num_vp >= Q {
				num_vp -= Q
				tot_vp += Q
				prel = Q
			} else {
				prel = num_vp
				num_vp = 0
				num_vm -= (Q - prel)
				tot_vp += prel
				tot_vm += (Q - prel)
			}
			richiesta.ack <- 1

			if tot_vm == TOT_P && tot_vp == TOT_P {
				running = false
			}

			fmt.Printf("[Deposito]: regione arancione prelevato %d vaccini VP e %d vaccini VM \n", prel, (Q - prel))
		case richiesta := <-when(running && num_vm+num_vp >= Q && len(prelievo[ARANCIONE]) == 0 && len(prelievo[ROSSA]) == 0, prelievo[GIALLA]):
			if num_vp >= Q {
				num_vp -= Q
				tot_vp += Q
				prel = Q
			} else {
				prel = num_vp
				num_vp = 0
				num_vm -= (Q - prel)
				tot_vp += prel
				tot_vm += (Q - prel)
			}
			richiesta.ack <- 1

			if tot_vm == TOT_P && tot_vp == TOT_P {
				running = false
			}

			fmt.Printf("[Deposito]: regione gialla prelevato %d vaccini VP e %d vaccini VM \n", prel, (Q - prel))
		case richiesta := <-when(!running, deposita[VP]):
			richiesta.ack <- -1
		case richiesta := <-when(!running, deposita[VM]):
			richiesta.ack <- -1
		case richiesta := <-when(!running, prenota[VM]):
			richiesta.ack <- -1
		case richiesta := <-when(!running, prenota[VM]):
			richiesta.ack <- -1
		case richiesta := <-when(!running, prelievo[ROSSA]):
			richiesta.ack <- -1
		case richiesta := <-when(!running, prelievo[ARANCIONE]):
			richiesta.ack <- -1
		case richiesta := <-when(!running, prelievo[GIALLA]):
			richiesta.ack <- -1
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

	var V1 int = 20

	go deposito()

	for i := 0; i < 2; i++ {
		go produttore(i, i)
	}

	for i := 0; i < V1; i++ {
		go regione(i)
	}

	// join
	for i := 0; i < V1+2; i++ {
		<-done
	}

	termina <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
