package main

import (
	"fmt"
	"math/rand"
	"time"
)

/////////////////////////////////////////////////////////////////////
// Costanti
/////////////////////////////////////////////////////////////////////
const NREG = 20
const maxP = 300
const maxM = 100
const tipoVP = 0
const tipoVM = 1
const NL = 40
const Q = 50
const Ncicli = 10
const Rossa = 0
const Arancione = 1
const Gialla = 2
const TOTP = 10
const TOTM = 8

var tipoVaccino = [2]string{"Pfizer-BionTech", "Moderna"}
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
var prelievo [3]chan int
var prenota [2]chan int
var consegna [2]chan int
var ack_case [2]chan int
var ack_regione [NREG]chan int

/////////////////////////////////////////////////////////////////////
// Join Goroutine
/////////////////////////////////////////////////////////////////////
var done = make(chan bool)
var termina = make(chan bool)
var termina_deposito = make(chan bool)

/////////////////////////////////////////////////////////////////////
// Funzioni Ausiliarie
/////////////////////////////////////////////////////////////////////
// se si usa struttura dati modificare la when
func when(b bool, c chan int) chan int {
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
func casa_farmaceutica(tipo int) {
	var fine int

	if tipo == tipoVP {
		fine = TOTP
	} else {
		fine = TOTM
	}

	for i := 0; i < fine; i++ {
		prenota[tipo] <- 1
		<-ack_case[tipo]

		fmt.Printf("[CasaFarmaceutica %s]: ho prenotato! \n", tipoVaccino[tipo])

		sleepRandTime(5)

		consegna[tipo] <- 1
		<-ack_case[tipo]

		fmt.Printf("[CasaFarmaceutica %s]: consegnato Lotto n. %d di %d dosi\n", tipoVaccino[tipo], i+1, NL)
	}

	done <- true
}

func regione(id int) {
	fmt.Printf("[Regione %s]: partenza\n", tipoRegione[id])

	for {
		zona := rand.Intn(3)

		fmt.Printf("[Regione %s]: è in zona %d\n", tipoRegione[id], zona)

		prelievo[zona] <- id
		esito := <-ack_regione[zona]

		if esito == -1 {
			fmt.Printf("[Regione %s]: termina\n", tipoRegione[id])
			done <- true
			return
		}

		fmt.Printf("[Regione %s]: prelevate %d dosi\n", tipoRegione[id], Q)

		sleepRandTime(5)
	}
}

func deposito() {
	var num_VP int = 0
	var num_VM int = 0
	var prenotati_VP = 0
	var prenotati_VM = 0
	var reg, VPprel int
	var fine bool = false

	for {
		select {
		case <-when((num_VP+prenotati_VP+NL < maxP), prenota[tipoVP]):
			prenotati_VP += NL
			ack_case[tipoVP] <- 1
			fmt.Printf("[deposito]: prenotato lotto VP, ci sono %d VP prenotati e %d VP disponibili\n", prenotati_VP, num_VP)
		case <-when((num_VM+prenotati_VM+NL < maxM), prenota[tipoVM]):
			prenotati_VM += NL
			ack_case[tipoVM] <- 1
			fmt.Printf("[deposito]: prenotato lotto VM, ci sono %d VM prenotati e %d VM disponibili\n", prenotati_VM, num_VM)
		case <-consegna[tipoVP]:
			prenotati_VP -= NL
			num_VP += NL
			ack_case[tipoVP] <- 1
			fmt.Printf("[deposito]: consegnato lotto VP: ora ci sono %d VP  e %d VM disponibili\n", num_VP, num_VM)
		case <-consegna[tipoVM]:
			prenotati_VM -= NL
			num_VM += NL
			ack_case[tipoVM] <- 1
			fmt.Printf("[deposito]: consegnato lotto VM: ora ci sono %d VP  e %d VM disponibili\n", num_VP, num_VM)
		case regione := <-when((fine == false && (num_VM+num_VP >= Q)), prelievo[Rossa]):
			if num_VP >= Q {
				num_VP -= Q
				VPprel = Q
			} else {
				VPprel = num_VP
				num_VP = 0
				num_VM -= (Q - VPprel)
			}
			ack_regione[regione] <- 1
			fmt.Printf("[deposito]: regione %s in zona ROSSA ha prelevato %d vaccini VP e %d vaccini VM \n", tipoRegione[reg], VPprel, (Q - VPprel))
		case regione := <-when((fine == false && (num_VM+num_VP >= Q) && len(prelievo[Rossa]) == 0), prelievo[Arancione]):
			if num_VP >= Q {
				num_VP -= Q
				VPprel = Q
			} else {
				VPprel = num_VP
				num_VP = 0
				num_VM -= (Q - VPprel)
			}
			ack_regione[regione] <- 1
			fmt.Printf("[deposito]: regione %s in zona ARANCIONE ha prelevato %d vaccini VP e %d vaccini VM \n", tipoRegione[reg], VPprel, (Q - VPprel))
		case regione := <-when((fine == false && (num_VM+num_VP >= Q) && len(prelievo[Rossa]) == 0 && len(prelievo[Arancione]) == 0), prelievo[Gialla]):
			if num_VP >= Q {
				num_VP -= Q
				VPprel = Q
			} else {
				VPprel = num_VP
				num_VP = 0
				num_VM -= (Q - VPprel)
			}
			ack_regione[regione] <- 1
			fmt.Printf("[deposito]: regione %s in zona GIALLA ha prelevato %d vaccini VP e %d vaccini VM \n", tipoRegione[reg], VPprel, (Q - VPprel))
		case regione := <-when(fine == true, prelievo[Rossa]):
			ack_regione[regione] <- -1
		case regione := <-when(fine == true, prelievo[Arancione]):
			ack_regione[regione] <- -1
		case regione := <-when(fine == true, prelievo[Gialla]):
			ack_regione[regione] <- -1
		case <-termina:
			fine = true
		case <-termina_deposito:
			fmt.Printf("[deposito]: termino (ci sono ancora %d dosi di VP e %d dosi di VM)\n", num_VP, num_VM)
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

	for i := 0; i < NREG; i++ {
		ack_regione[i] = make(chan int, 100)
	}

	for i := 0; i < 3; i++ {
		prelievo[i] = make(chan int, 100)
	}

	for i := 0; i < 2; i++ {
		prenota[i] = make(chan int, 100)
		consegna[i] = make(chan int, 100)
		ack_case[i] = make(chan int, 100)
	}

	go deposito()
	go casa_farmaceutica(0)
	go casa_farmaceutica(1)

	for i := 0; i < NREG; i++ {
		go regione(i)
	}

	for i := 0; i < 2; i++ {
		<-done
	}

	termina <- true

	for i := 0; i < NREG; i++ {
		<-done
	}

	termina_deposito <- true
	<-done
}
