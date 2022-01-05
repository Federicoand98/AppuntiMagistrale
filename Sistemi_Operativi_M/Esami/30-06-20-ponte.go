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
const MAX = 5
const TA = 0
const TB = 1
const PUBBLICO = 0
const PRIVATO = 0
const DESTRA = 0
const SINISTRA = 1

var tipoVeicolo = [2]string{"Pubblico", "Privato"}
var tipoStato = [2]string{"TA", "TB"}

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
var entrata_destra [2]chan Richiesta
var entrata_sinistra [2]chan Richiesta
var uscita_destra [2]chan Richiesta
var uscita_sinistra [2]chan Richiesta
var richiesta_attraversamento = make(chan Richiesta, MAXBUFF)
var termine_attraversamento = make(chan Richiesta, MAXBUFF)

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
func autoveicolo(id int, tipo int) {
	richiesta := Richiesta{id, make(chan int)}
	direzione := rand.Intn(2)

	if direzione == DESTRA {
		fmt.Printf("[Autoveicolo %s %d]: avviato a destra\n", tipoVeicolo[tipo], id)

		entrata_destra[tipo] <- richiesta
		<-richiesta.ack
		fmt.Printf("[Autoveicolo %s %d]: entrato a destra, sta attraversando il ponte\n", tipoVeicolo[tipo], id)

		sleepRandTime(5)

		uscita_sinistra[tipo] <- richiesta
		<-richiesta.ack
		fmt.Printf("[Autoveicolo %s %d]: ponte attraversato ed uscito a sinistra\n", tipoVeicolo[tipo], id)
	} else if direzione == SINISTRA {
		fmt.Printf("[Autoveicolo %s %d]: avviato a sinistra\n", tipoVeicolo[tipo], id)

		entrata_sinistra[tipo] <- richiesta
		<-richiesta.ack
		fmt.Printf("[Autoveicolo %s %d]: entrato a sinistra, sta attraversando il ponte\n", tipoVeicolo[tipo], id)

		sleepRandTime(5)

		uscita_destra[tipo] <- richiesta
		<-richiesta.ack
		fmt.Printf("[Autoveicolo %s %d]: ponte attraversato ed uscito a destra\n", tipoVeicolo[tipo], id)
	}

	done <- true
}

func imbarcazione(id int) {
	richiesta := Richiesta{id, make(chan int)}

	sleepRandTime(id)

	fmt.Printf("[Imbarcazione %d]: avviata\n", id)

	richiesta_attraversamento <- richiesta
	<-richiesta.ack
	fmt.Printf("[Imbarcazione %d]: sto attraversando\n", id)

	sleepRandTime(5)

	termine_attraversamento <- richiesta
	<-richiesta.ack
	fmt.Printf("[Imbarcazione %d]: fine attraversamento\n", id)

	<-done
}

func ponte() {
	var stato int = rand.Intn(2)
	var num_auto_d int = 0
	var num_auto_s int = 0
	var fiume_libero bool = true

	for {
		select {
		case richiesta := <-when((stato == TB && fiume_libero) || (stato == TA && num_auto_s == 0 && num_auto_d == 0), richiesta_attraversamento):
			if stato == TA {
				stato = TB
				fmt.Printf("[Ponte]: transizione ponte TA->TB\n")
			}
			fiume_libero = false
			richiesta.ack <- 1
			fmt.Printf("[Ponte]: la nave %d sta attraversando il ponte. Stato: %s\n", richiesta.id, tipoStato[stato])
		case richiesta := <-when(stato == TB, termine_attraversamento):
			if len(richiesta_attraversamento) == 0 {
				stato = TA
				fmt.Printf("[Ponte]: transizione ponte TB->TA\n")
			}
			fiume_libero = true
			richiesta.ack <- 1
			fmt.Printf("[Ponte]: la nave %d ha attraversato il ponte. Stato: %s\n", richiesta.id, tipoStato[stato])
		case richiesta := <-when(((stato == TA && num_auto_d < MAX && num_auto_s == 0) || (stato == TB && fiume_libero && len(richiesta_attraversamento) == 0)), entrata_destra[PRIVATO]):
			if stato == TB {
				stato = TA
				fmt.Printf("[Ponte]: transizione ponte TB->TA\n")
			}
			num_auto_d++
			richiesta.ack <- 1
			fmt.Printf("[Ponte]: auto privata %d entra da destra. Stato: %s. Num_d: %d. Num_s: %d\n", richiesta.id, tipoStato[stato], num_auto_d, num_auto_s)
		case richiesta := <-when(((stato == TA && num_auto_d < MAX && num_auto_s == 0 && len(entrata_destra[PRIVATO]) == 0) || (stato == TB && fiume_libero && len(richiesta_attraversamento) == 0)), entrata_destra[PUBBLICO]):
			if stato == TB {
				stato = TA
				fmt.Printf("[Ponte]: transizione ponte TB->TA\n")
			}
			num_auto_d++
			richiesta.ack <- 1
			fmt.Printf("[Ponte]: auto pubblica %d entra da destra. Stato: %s. Num_d: %d. Num_s: %d\n", richiesta.id, tipoStato[stato], num_auto_d, num_auto_s)
		case richiesta := <-when(((stato == TA && num_auto_s < MAX && num_auto_d == 0) || (stato == TB && fiume_libero && len(richiesta_attraversamento) == 0)), entrata_sinistra[PRIVATO]):
			if stato == TB {
				stato = TA
				fmt.Printf("[Ponte]: transizione ponte TB->TA\n")
			}
			num_auto_s++
			richiesta.ack <- 1
			fmt.Printf("[Ponte]: auto privata %d entra da sinistra. Stato: %s. Num_d: %d. Num_s: %d\n", richiesta.id, tipoStato[stato], num_auto_d, num_auto_s)
		case richiesta := <-when(((stato == TA && num_auto_s < MAX && num_auto_d == 0 && len(entrata_sinistra[PRIVATO]) == 0) || (stato == TB && fiume_libero && len(richiesta_attraversamento) == 0)), entrata_sinistra[PUBBLICO]):
			if stato == TB {
				stato = TA
				fmt.Printf("[Ponte]: transizione ponte TB->TA\n")
			}
			num_auto_s++
			richiesta.ack <- 1
			fmt.Printf("[Ponte]: auto pubblica %d entra da sinistra. Stato: %s. Num_d: %d. Num_s: %d\n", richiesta.id, tipoStato[stato], num_auto_d, num_auto_s)
		case richiesta := <-when(stato == TA, uscita_destra[PRIVATO]):
			num_auto_s--
			richiesta.ack <- 1
			fmt.Printf("[Ponte]: auto privata %d uscita da destra. Stato: %s. Num_d: %d. Num_s: %d\n", richiesta.id, tipoStato[stato], num_auto_d, num_auto_s)
		case richiesta := <-when(stato == TA, uscita_destra[PUBBLICO]):
			num_auto_s--
			richiesta.ack <- 1
			fmt.Printf("[Ponte]: auto pubblica %d uscita da destra. Stato: %s. Num_d: %d. Num_s: %d\n", richiesta.id, tipoStato[stato], num_auto_d, num_auto_s)
		case richiesta := <-when(stato == TA, uscita_sinistra[PRIVATO]):
			num_auto_d--
			richiesta.ack <- 1
			fmt.Printf("[Ponte]: auto pubblica %d uscita da sinistra. Stato: %s. Num_d: %d. Num_s: %d\n", richiesta.id, tipoStato[stato], num_auto_d, num_auto_s)
		case richiesta := <-when(stato == TA, uscita_sinistra[PUBBLICO]):
			num_auto_d--
			richiesta.ack <- 1
			fmt.Printf("[Ponte]: auto pubblica %d uscita da sinistra. Stato: %s. Num_d: %d. Num_s: %d\n", richiesta.id, tipoStato[stato], num_auto_d, num_auto_s)
		case <-termina:
			fmt.Println("\n\n[Ponte]: Fine!")
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

	var N_AUTO int = 30
	var N_BARCHE int = 5

	//inizializzazione canali se non si usa struttura dati
	for i := 0; i < 2; i++ {
		entrata_destra[i] = make(chan Richiesta, MAXBUFF)
		entrata_sinistra[i] = make(chan Richiesta, MAXBUFF)
		uscita_destra[i] = make(chan Richiesta, MAXBUFF)
		uscita_sinistra[i] = make(chan Richiesta, MAXBUFF)
	}

	go ponte()

	for i := 0; i < N_AUTO; i++ {
		go autoveicolo(i, rand.Intn(2))
	}

	for i := 0; i < N_BARCHE; i++ {
		go imbarcazione(i)
	}

	// join
	for i := 0; i < N_AUTO+N_BARCHE; i++ {
		<-done
	}

	termina <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
