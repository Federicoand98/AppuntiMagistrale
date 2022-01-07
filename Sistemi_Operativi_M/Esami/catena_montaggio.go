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
const MAX = 100
const MODELLO_A = 0
const MODELLO_B = 1
const PA = 0
const PB = 1
const CA = 2
const CB = 3
const MAX_P = 3
const MAX_C = 3

var tipoNastro = [4]string{"PA", "PB", "CA", "CB"}
var tipoRobot = [2]string{"A", "B"}

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
var deposito [4]chan Richiesta
var prelievo [4]chan Richiesta

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
func nastro(tipo int) {
	richiesta := Richiesta{0, make(chan int)} // se si usa struttura
	fmt.Printf("[Nastro %s]: avviato\n", tipoNastro[tipo])

	for {
		deposito[PA] <- richiesta
		ris := <-richiesta.ack

		if ris == -1 {
			fmt.Printf("[Nastro %s]: termino\n", tipoNastro[tipo])
			done <- true
			return
		}

		fmt.Printf("[Nastro %s]: depositato %s\n", tipoNastro[tipo], tipoNastro[tipo])

		sleepRandTime(3)
	}
}

func robot(tipo int) {
	richiesta := Richiesta{0, make(chan int)}
	fmt.Printf("[Robot %s]: avviato\n", tipoRobot[tipo])
	var ris int

	for {
		for i := 0; i < 4; i++ {
			if tipo == MODELLO_A {
				prelievo[CA] <- richiesta
				ris = <-richiesta.ack

				if ris == -1 {
					fmt.Printf("[Robot %s]: termino", tipoRobot[tipo])
				}

				fmt.Printf("[Robot %s]: prelevato CA\n", tipoRobot[tipo])

				prelievo[PA] <- richiesta
				ris = <-richiesta.ack

				if ris == -1 {
					fmt.Printf("[Robot %s]: termino", tipoRobot[tipo])
				}

				fmt.Printf("[Robot %s]: prelevato PA\n", tipoRobot[tipo])
			} else if tipo == MODELLO_B {
				prelievo[CB] <- richiesta
				ris = <-richiesta.ack

				if ris == -1 {
					fmt.Printf("[Robot %s]: termino", tipoRobot[tipo])
				}

				fmt.Printf("[Robot %s]: prelevato CB\n", tipoRobot[tipo])

				prelievo[PB] <- richiesta
				ris = <-richiesta.ack

				if ris == -1 {
					fmt.Printf("[Robot %s]: termino", tipoRobot[tipo])
				}

				fmt.Printf("[Robot %s]: prelevato PB\n", tipoRobot[tipo])
			}

			sleepRandTime(2)
		}

		sleepRandTime(3)
	}
}

func Deposito() {
	var fine bool = false
	var num_ca int = 0
	var num_pa int = 0
	var num_cb int = 0
	var num_pb int = 0
	var montati_ca int = 0
	var montati_cb int = 0
	var montati_pa int = 0
	var montati_pb int = 0
	var montate_a int = 0
	var montate_b int = 0
	var tot int = 20

	for {
		select {
		case richiesta := <-when(!fine && num_ca+num_cb < MAX_C && (montate_a < montate_b || (montate_a >= montate_b && len(deposito[CB]) == 0)), deposito[CA]):
			num_ca++
			richiesta.ack <- 1
			fmt.Printf("[Deposito]: depositato CA. NumCA: %d, NumCB: %d, NumPA: %d, NumPB: %d, NumMontateA: %d, NumMontateB: %d\n", num_ca, num_cb, num_pa, num_pb, montate_a, montate_b)
		case richiesta := <-when(!fine && num_ca+num_cb < MAX_C && (montate_b < montate_a || (montate_b >= montate_a && len(deposito[CA]) == 0)), deposito[CB]):
			num_cb++
			richiesta.ack <- 1
			fmt.Printf("[Deposito]: depositato CB. NumCA: %d, NumCB: %d, NumPA: %d, NumPB: %d, NumMontateA: %d, NumMontateB: %d\n", num_ca, num_cb, num_pa, num_pb, montate_a, montate_b)
		case richiesta := <-when(!fine && num_pa+num_pb < MAX_P && (montate_a < montate_b || (montate_a >= montate_b && len(deposito[PB]) == 0)), deposito[PA]):
			num_pa++
			richiesta.ack <- 1
			fmt.Printf("[Deposito]: depositato PA. NumCA: %d, NumCB: %d, NumPA: %d, NumPB: %d, NumMontateA: %d, NumMontateB: %d\n", num_ca, num_cb, num_pa, num_pb, montate_a, montate_b)
		case richiesta := <-when(!fine && num_pa+num_pb < MAX_P && (montate_b < montate_a || (montate_b >= montate_a && len(deposito[PA]) == 0)), deposito[PB]):
			num_pb++
			richiesta.ack <- 1
			fmt.Printf("[Deposito]: depositato PB. NumCA: %d, NumCB: %d, NumPA: %d, NumPB: %d, NumMontateA: %d, NumMontateB: %d\n", num_ca, num_cb, num_pa, num_pb, montate_a, montate_b)
		case richiesta := <-when(!fine && num_ca > 0 && (montate_a < montate_b || (montate_a >= montate_b && len(prelievo[CB]) == 0)), prelievo[CA]):
			num_ca--
			montati_ca++
			richiesta.ack <- 1
			fmt.Printf("[Deposito]: montato CA. NumCA: %d, NumCB: %d, NumPA: %d, NumPB: %d, NumMontateA: %d, NumMontateB: %d\n", num_ca, num_cb, num_pa, num_pb, montate_a, montate_b)
		case richiesta := <-when(!fine && num_cb > 0 && (montate_b < montate_a || (montate_b >= montate_a && len(prelievo[CA]) == 0)), prelievo[CB]):
			num_cb--
			montati_cb++
			richiesta.ack <- 1
			fmt.Printf("[Deposito]: montato CB. NumCA: %d, NumCB: %d, NumPA: %d, NumPB: %d, NumMontateA: %d, NumMontateB: %d\n", num_ca, num_cb, num_pa, num_pb, montate_a, montate_b)
		case richiesta := <-when(!fine && num_pa > 0 && (montate_a < montate_b || (montate_a >= montate_b && len(prelievo[PB]) == 0)), prelievo[PA]):
			num_pa--
			montati_pa++
			richiesta.ack <- 1
			fmt.Printf("[Deposito]: montato PA. NumCA: %d, NumCB: %d, NumPA: %d, NumPB: %d, NumMontateA: %d, NumMontateB: %d\n", num_ca, num_cb, num_pa, num_pb, montate_a, montate_b)
		case richiesta := <-when(!fine && num_pb > 0 && (montate_b < montate_a || (montate_b >= montate_a && len(prelievo[CA]) == 0)), prelievo[PB]):
			num_pb--
			montati_pb++
			richiesta.ack <- 1
			fmt.Printf("[Deposito]: montato PB. NumCA: %d, NumCB: %d, NumPA: %d, NumPB: %d, NumMontateA: %d, NumMontateB: %d\n", num_ca, num_cb, num_pa, num_pb, montate_a, montate_b)
		case richiesta := <-when(fine, deposito[CA]):
			richiesta.ack <- -1
		case richiesta := <-when(fine, deposito[CB]):
			richiesta.ack <- -1
		case richiesta := <-when(fine, deposito[PA]):
			richiesta.ack <- -1
		case richiesta := <-when(fine, deposito[PB]):
			richiesta.ack <- -1
		case richiesta := <-when(fine, prelievo[CA]):
			richiesta.ack <- -1
		case richiesta := <-when(fine, prelievo[CB]):
			richiesta.ack <- -1
		case richiesta := <-when(fine, prelievo[PA]):
			richiesta.ack <- -1
		case richiesta := <-when(fine, prelievo[PB]):
			richiesta.ack <- -1
		case <-termina:
			fmt.Println("\n\n[SERVER]: Fine!")
			done <- true
			return
		}

		if montati_ca == 4 && montati_pa == 4 {
			montate_a++
			montati_ca = 0
			montati_pa = 0
		}

		if montati_cb == 4 && montati_pb == 4 {
			montate_b++
			montati_cb = 0
			montati_pb = 0
		}

		if montate_a+montate_b == tot {
			fine = true
		}
	}
}

/////////////////////////////////////////////////////////////////////
// Main
/////////////////////////////////////////////////////////////////////
func main() {
	rand.Seed(time.Now().Unix())

	go Deposito()

	for i := 0; i < 4; i++ {
		go nastro(i)
	}

	for i := 0; i < 2; i++ {
		go robot(i)
	}

	for i := 0; i < 4; i++ {
		<-done
	}

	for i := 0; i < 2; i++ {
		<-done
	}

	termina <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
