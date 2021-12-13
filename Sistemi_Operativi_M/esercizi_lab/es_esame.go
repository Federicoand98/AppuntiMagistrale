package main

import (
	"fmt"
	"math/rand"
	"time"
)

const maxP = 3
const maxC = 3
const tipoPA = 0
const tipoPB = 1
const tipoCA = 2
const tipoCB = 3
const robotA = 0
const robotB = 1
const TOT = 10 // max numero auto da montare

var tipoRobot = [2]string{"Modello A", "Modello B"}
var tipoNastro = [4]string{"Pneumatico A", "Pneumatico B", "Cerchio A", "Cerchio B"}

var done = make(chan bool)
var terminaDeposito = make(chan bool)

var prelievoPA = make(chan int, 100)
var prelievoPB = make(chan int, 100)
var prelievoCA = make(chan int, 100)
var prelievoCB = make(chan int, 100)

var depositoPA = make(chan int, 100)
var depositoPB = make(chan int, 100)
var depositoCA = make(chan int, 100)
var depositoCB = make(chan int, 100)

var ack_robotA = make(chan int)
var ack_robotB = make(chan int)
var ack_nastroPA = make(chan int)
var ack_nastroPB = make(chan int)
var ack_nastroCA = make(chan int)
var ack_nastroCB = make(chan int)

func Robot(tipo int) {
	var tt int
	var k = 0
	var ris int

	fmt.Printf("[Robot %s]: partenza!\n", tipoRobot[tipo])

	for {
		for i := 0; i < 4; i++ {
			if tipo == robotA {
				prelievoCA <- tipo
				ris = <-ack_robotA

				if ris == -1 {
					fmt.Printf("[Robot %s]: termino...", tipoRobot[tipo])
					done <- true
					return
				}

				fmt.Printf("[Robot %s]: prelevato cerchio CA\n", tipoRobot[tipo])

				tt = (rand.Intn(2) + 1)
				time.Sleep(time.Duration(tt) * time.Second)

				prelievoPA <- tipo
				ris = <-ack_robotA

				if ris == -1 {
					fmt.Printf("[Robot %s]: termino...", tipoRobot[tipo])
					done <- true
					return
				}

				fmt.Printf("[Robot %s]: prelevato prenumatico PA\n", tipoRobot[tipo])

				tt = (rand.Intn(2) + 1)
				time.Sleep(time.Duration(tt) * time.Second)
			} else {
				prelievoCB <- tipo
				ris = <-ack_robotB

				if ris == -1 {
					fmt.Printf("[Robot %s]: termino...", tipoRobot[tipo])
					done <- true
					return
				}

				fmt.Printf("[Robot %s]: prelevato cerchio CB\n", tipoRobot[tipo])

				tt = (rand.Intn(2) + 1)
				time.Sleep(time.Duration(tt) * time.Second)

				prelievoPB <- tipo
				ris = <-ack_robotB

				if ris == -1 {
					fmt.Printf("[Robot %s]: termino...", tipoRobot[tipo])
					done <- true
					return
				}

				fmt.Printf("[Robot %s]: prelevato prenumatico PB\n", tipoRobot[tipo])

				tt = (rand.Intn(2) + 1)
				time.Sleep(time.Duration(tt) * time.Second)
			}
		}

		k++
		fmt.Printf("[Robot %s]: auto numero %d terminata.\n", tipoRobot[tipo], k)
	}
}

func Nastro(tipo int) {
	var tt int
	var i int
	var ris int

	for {
		tt = rand.Intn(2) + 1
		time.Sleep(time.Duration(tt) * time.Second)

		switch {
		case tipo == tipoPA:
			depositoPA <- 1
			ris = <-ack_nastroPA

			if ris == -1 {
				fmt.Printf("[Nastro %s]: termino...\n", tipoNastro[tipo])
				done <- true
				return
			}

			fmt.Printf("[Nastro %s]: consegnato %s\n", tipoNastro[tipo], tipoNastro[tipo])
		case tipo == tipoPB:
			depositoPB <- 1
			ris = <-ack_nastroPB

			if ris == -1 {
				fmt.Printf("[Nastro %s]: termino...\n", tipoNastro[tipo])
				done <- true
				return
			}

			fmt.Printf("[Nastro %s]: consegnato %s\n", tipoNastro[tipo], tipoNastro[tipo])
		case tipo == tipoCA:
			depositoCA <- 1
			ris = <-ack_nastroCA

			if ris == -1 {
				fmt.Printf("[Nastro %s]: termino...\n", tipoNastro[tipo])
				done <- true
				return
			}

			fmt.Printf("[Nastro %s]: consegnato %s\n", tipoNastro[tipo], tipoNastro[tipo])
		case tipo == tipoCB:
			depositoCB <- 1
			ris = <-ack_nastroCB

			if ris == -1 {
				fmt.Printf("[Nastro %s]: termino...\n", tipoNastro[tipo])
				done <- true
				return
			}

			fmt.Printf("[Nastro %s]: consegnato %s\n", tipoNastro[tipo], tipoNastro[tipo])
		}

		i++
	}
}

func Deposito() {
	var numCAMontati int = 0
	var numCBMontati int = 0
	var numPAMontati int = 0
	var numPBMontati int = 0

	var numAMontati int = 0
	var numBMontati int = 0

	var numCA int = 0
	var numCB int = 0
	var numPA int = 0
	var numPB int = 0

	var totP int = 0
	var totC int = 0
	var fine bool = false

	for {
		select {
		case <-when(fine == false && (totC < maxC && numCA < maxC-1) && (numAMontati < numBMontati || (numAMontati >= numBMontati && len(depositoCB) == 0)), depositoCA):
			numCA++
			totC++
			ack_nastroCA <- 1
			fmt.Printf("[Deposito]: aggiunto cerchio A, ci sono %d CA e %d CB -> tot cerchi nel deposito = %d\n", numCA, numCB, totC)

		case <-when(fine == false && (totC < maxC && numCB < maxC-1) && (numAMontati < numBMontati || (numAMontati < numBMontati && len(depositoCA) == 0)), depositoCB):
			numCB++
			totC++
			ack_nastroCB <- 1
			fmt.Printf("[Deposito]: aggiunto cerchio B, ci sono %d CA e %d CB -> tot cerchi nel deposito = %d\n", numCA, numCB, totC)

		case <-when(fine == false && (totP < maxP && numPA < maxP-1) && (numAMontati < numBMontati || (numAMontati >= numBMontati && len(depositoPB) == 0)), depositoPA):
			numPA++
			totP++
			ack_nastroPA <- 1
			fmt.Printf("[Deposito]: aggiunto pneumatico A, ci sono %d PA e %d PB -> tot pneumatici nel deposito = %d\n", numPA, numPB, totP)

		case <-when(fine == false && (totP < maxP && numPB < maxP-1) && (numAMontati >= numBMontati || (numAMontati < numBMontati && len(depositoPA) == 0)), depositoPB):
			numPB++
			totP++
			ack_nastroPB <- 1
			fmt.Printf("[Deposito]: aggiunto pneumatico B, ci sono %d PA e %d PB -> tot pneumatici nel deposito = %d\n", numPA, numPB, totP)

		case <-when(fine == false && numCA > 0 && (numAMontati < numBMontati || (numAMontati >= numBMontati && len(prelievoCB) == 0)), prelievoCA):
			numCA--
			totC--
			numCAMontati++
			ack_robotA <- 1
			fmt.Printf("[Deposito]: prelevato cerchio A, tot cerchi nel deposito = %d\n", totC)

		case <-when(fine == false && numCB > 0 && (numAMontati >= numBMontati || (numAMontati < numBMontati && len(prelievoCA) == 0)), prelievoCB):
			numCB--
			totC--
			numCBMontati++
			ack_robotB <- 1
			fmt.Printf("[Deposito]: prelevato cerchio B, tot cerchi nel deposito = %d\n", totC)

		case <-when(fine == false && numPA > 0 && (numAMontati < numBMontati || (numAMontati >= numBMontati && len(prelievoPB) == 0)), prelievoPA):
			numPA--
			totP--
			numPAMontati++
			ack_robotA <- 1
			fmt.Printf("[Deposito]: prelevato pneumatico A, tot pneumatici nel deposito = %d\n", totC)

		case <-when(fine == false && numPB > 0 && (numAMontati >= numBMontati || (numAMontati < numBMontati && len(prelievoPA) == 0)), prelievoPB):
			numPB--
			totP--
			numPBMontati++
			ack_robotB <- 1
			fmt.Printf("[Deposito]: prelevato pneumatico B, tot pneumatici nel deposito = %d\n", totC)

		case <-when(fine == true, depositoCA):
			ack_nastroCA <- -1

		case <-when(fine == true, depositoCB):
			ack_nastroCB <- -1

		case <-when(fine == true, depositoPA):
			ack_nastroPA <- -1

		case <-when(fine == true, depositoPB):
			ack_nastroPB <- -1

		case <-when(fine == true, prelievoCA):
			ack_robotA <- -1

		case <-when(fine == true, prelievoCB):
			ack_robotB <- -1

		case <-when(fine == true, prelievoPA):
			ack_robotA <- -1

		case <-when(fine == true, prelievoPB):
			ack_robotB <- -1

		case <-terminaDeposito:
			fmt.Printf("[Deposito]: termino\n")
			done <- true
			return
		}

		if numCAMontati == 4 && numPAMontati == 4 {
			numAMontati++
			numPAMontati = 0
			numCAMontati = 0
		}

		if numCBMontati == 4 && numPBMontati == 4 {
			numBMontati++
			numCBMontati = 0
			numPBMontati = 0
		}

		fmt.Printf("[Deposito] montate A = %d. montate B = %d\n", numAMontati, numBMontati)

		if numAMontati+numBMontati == TOT {
			fine = true
		}
	}
}

func when(b bool, c chan int) chan int {
	if !b {
		return nil
	}

	return c
}

func main() {
	rand.Seed(time.Now().Unix())
	fmt.Printf("[Main] attivo 4 nastri trasportatori e 2 robot\n")

	go Deposito()

	for i := 0; i < 4; i++ {
		go Nastro(i)
	}

	for i := 0; i < 2; i++ {
		go Robot(i)
	}

	for i := 0; i < 4; i++ {
		<-done
	}

	for i := 0; i < 2; i++ {
		<-done
	}

	terminaDeposito <- true
	<-done

	fmt.Printf("[Main] APPLICAZIONE TERMINATA \n")
}
