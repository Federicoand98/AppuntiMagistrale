package main

import (
	"fmt"
	"math/rand"
	"time"
)

/////////////////////////////////////////////////////////////////////
// Costanti
/////////////////////////////////////////////////////////////////////
const MAXBUFF int = 100
const MAX int = 18
const N_COMMESSI int = 8
const N_CLIENTI int = 70
const NM = 10
const OCCASIONALI = 0
const ABITUALI = 1

var tipoClienteStr [2]string = [2]string{"ABITUALE", "OCCASIONALE"}

/////////////////////////////////////////////////////////////////////
// Strutture Dati
/////////////////////////////////////////////////////////////////////
type Richiesta struct {
	id  int
	ack chan bool
}

type Commesso struct {
	dentro                   bool
	vuole_uscire             bool
	clienti_assegnati        [3]int
	numero_clienti_assegnati int
	ack_uscita               chan bool
}

/////////////////////////////////////////////////////////////////////
// Canali
/////////////////////////////////////////////////////////////////////
var entrata_clienti_abituali = make(chan Richiesta, MAXBUFF)
var entrata_clienti_occasionali = make(chan Richiesta, MAXBUFF)
var uscita_clienti = make(chan Richiesta)
var entrata_commessi = make(chan Richiesta, MAXBUFF)
var uscita_commessi = make(chan Richiesta, MAXBUFF)
var deposita_mascherine = make(chan bool)

/////////////////////////////////////////////////////////////////////
// Join Goroutine
/////////////////////////////////////////////////////////////////////
var done = make(chan bool)
var termina_cliente = make(chan bool)
var terminaCommesso = make([]chan bool, N_COMMESSI)
var terminaFornitore = make(chan bool)
var terminaNegozio = make(chan bool)

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
func cliente(id int, tipo int) {
	richiesta := Richiesta{id, make(chan bool)}

	sleepRandTime(3)

	fmt.Printf("[CLIENTE %s %d] Voglio entrare nel negozio...\n", tipoClienteStr[tipo], id)

	if tipo == OCCASIONALI {
		entrata_clienti_occasionali <- richiesta
		<-richiesta.ack
		fmt.Printf("[CLIENTE %s %d] Sono entrato nel negozio...\n", tipoClienteStr[tipo], id)
	} else if tipo == ABITUALI {
		entrata_clienti_abituali <- richiesta
		<-richiesta.ack
		fmt.Printf("[CLIENTE %s %d] Sono entrato nel negozio...\n", tipoClienteStr[tipo], id)
	}

	sleepRandTime(5)

	if tipo == OCCASIONALI {
		uscita_clienti_occasionali <- richiesta
		<-richiesta.ack
		fmt.Printf("[CLIENTE %s %d] Sono uscito dal negozio...\n", tipoClienteStr[tipo], id)
	} else if tipo == ABITUALI {
		uscita_clienti_occasionali <- richiesta
		<-richiesta.ack
		fmt.Printf("[CLIENTE %s %d] Sono uscito dal negozio...\n", tipoClienteStr[tipo], id)
	}

	termina_cliente <- true
	return
}

func commesso(id int, termina chan bool, done chan bool) {
	richiesta := Richiesta{id, make(chan bool)}

	for {
		sleepRandTime(2)

		fmt.Printf("[COMMESSO %d] Voglio entrare nel negozio...\n", id)
		entrata_commessi <- richiesta
		<-richiesta.ack

		fmt.Printf("[COMMESSO %d] Sono entrato nel negozio...\n", id)

		sleepRandTime(7)

		uscita_commessi <- richiesta
		<-richiesta.ack

		fmt.Printf("[COMMESSO %d] Sono uscito dal negozio...\n", id)

		select {
		case <-termina:
			{
				fmt.Printf("[COMMESSO %d] Termino...\n", id)
				done <- true
				return
			}
		default:
			{
				sleepRandTime(2)
			}
		}
	}
}

func fornitore() {
	for {
		sleepRandTime(5)

		fmt.Printf("[FORNITORE] Voglio consegnare il lotto di mascherine...\n")

		deposita_mascherine <- true
		<-deposita_mascherine

		fmt.Printf("[FORNITORE] Consegna effettuata...\n")

		select {
		case <-terminaFornitore:
			{
				fmt.Printf("[FORNITORE] Termino...\n")
				terminaFornitore <- true
				return
			}
		default:
			{
				sleepRandTime(2)
			}
		}
	}
}

func negozio() {
	var clienti_dentro = 0
	var commessi_dentro = 0
	var commessi_liberi = 0
	var mascherine = 0
	commessi := make([]Commesso, N_COMMESSI)

	for i := 0; i < N_COMMESSI; i++ {
		commessi[i].dentro = false
		commessi[i].vuole_uscire = false
		commessi[i].numero_clienti_assegnati = 0
		commessi[i].ack_uscita = nil

		for j := 0; j < 3; j++ {
			commessi[i].clienti_assegnati[j] = -1
		}
	}

	fmt.Printf("MAX: %d, NM: %d, N_CLIENTI: %d, N_COMMESSI: %d...\n", MAX, NM, N_CLIENTI, N_COMMESSI)

	for {
		fmt.Printf("[NEGOZIO] ClientiDentro: %d, CommessiDentro: %d, CommessiLiberi: %d, Mascherine: %d...\n", clienti_dentro, commessi_dentro, commessi_liberi, mascherine)

		select {
		case <-deposita_mascherine:
			{
				mascherine += NM
				fmt.Printf("[NEGOZIO] Il fornitore ha depositato %d mascherine...\n", NM)
				deposita_mascherine <- true
			}
		case richiesta := <-when(true, entrata_commessi):
			{
				commessi_dentro++
				commessi_liberi++
				commessi[richiesta.id].dentro = true
				commessi[richiesta.id].vuole_uscire = false
				commessi[richiesta.id].numero_clienti_assegnati = 0

				for i := 0; i < 3; i++ {
					commessi[richiesta.id].clienti_assegnati[i] = -1
				}

				fmt.Printf("[NEGOZIO] Il commesso %d entra nel negozio...\n", richiesta.id)
				richiesta.ack <- true
			}
		case richiesta := <-uscita_commessi:
			{
				if commessi[richiesta.id].numero_clienti_assegnati == 0 {
					fmt.Printf("[NEGOZIO] Il commesso %d esce dal negozio...\n", richiesta.id)
					commessi_dentro--
					commessi_liberi--
					commessi[richiesta.id].dentro = false
					commessi[richiesta.id].vuole_uscire = false
					commessi[richiesta.id].ack_uscita = nil
					richiesta.ack <- true
				} else {
					fmt.Printf("[NEGOZIO] Il commesso %d è in attesa di uscire dal negozio (%d clienti assegnati)...\n", richiesta.id, commessi[richiesta.id].numero_clienti_assegnati)
					commessi[richiesta.id].vuole_uscire = true
					commessi[richiesta.id].ack_uscita = richiesta.ack
				}
			}
		case richiesta := <-when((commessi_dentro > 0 && commessi_liberi > 0 && mascherine >= 1 && len(entrata_commessi) == 0 && clienti_dentro+commessi_dentro < MAX), entrata_clienti_abituali):
			{
				found := false

				for i := 0; i < N_COMMESSI && !found; i++ {
					if commessi[i].dentro && commessi[i].numero_clienti_assegnati < 3 {
						for j := 0; j < 3 && !found; j++ {
							if commessi[i].clienti_assegnati[j] < 0 {
								commessi[i].clienti_assegnati[j] = richiesta.id
								commessi[i].numero_clienti_assegnati++

								if commessi[i].numero_clienti_assegnati == 3 {
									commessi_liberi--
								}

								clienti_dentro++
								mascherine--
								found = true
								richiesta.ack <- true

								fmt.Printf("[NEGOZIO] Il cliente ABITUALE %d entra nel negozio...\n", richiesta.id)
								fmt.Printf("[NEGOZIO] Assegno il commesso %d al cliente ABITUALE %d...\n", i, richiesta.id)
							}
						}
					}
				}
			}
		case richiesta := <-when((commessi_dentro > 0 && commessi_liberi > 0 && mascherine >= 1 && len(entrata_commessi) == 0 && len(entrata_clienti_abituali) == 0 && clienti_dentro+commessi_dentro < MAX), entrata_clienti_occasionali):
			{
				found := false
				for i := 0; i < N_COMMESSI && !found; i++ {
					if commessi[i].dentro && commessi[i].numero_clienti_assegnati < 3 {
						for j := 0; j < 3 && !found; j++ {
							if commessi[i].clienti_assegnati[j] < 0 {
								commessi[i].clienti_assegnati[j] = richiesta.id // assegno l'id del cliente
								commessi[i].numero_clienti_assegnati++

								if commessi[i].numero_clienti_assegnati == 3 {
									commessi_liberi--
								}

								commessi_dentro++
								mascherine--
								found = true
								richiesta.ack <- true
								fmt.Printf("[NEGOZIO] Il cliente OCCASIONALE %d entra nel negozio...\n", richiesta.id)
								fmt.Printf("[NEGOZIO] Assegno il commesso %d al cliente OCCASIONALE %d...\n", i, richiesta.id)
							}
						}
					}
				}
			}

		case richiesta := <-uscita_clienti:
			{
				found := false

				for i := 0; i < N_COMMESSI && !found; i++ {
					if commessi[i].dentro {
						for j := 0; j < 3 && !found; j++ {
							if commessi[i].clienti_assegnati[j] == richiesta.id {
								found = true
								commessi[i].clienti_assegnati[j] = -1

								if commessi[i].clienti_assegnati[j] == 3 {
									commessi_liberi++
								}

								commessi[i].numero_clienti_assegnati--
								clienti_dentro--

								fmt.Printf("[NEGOZIO] Il cliente %d esce dal negozio...\n", richiesta.id)
								fmt.Printf("[NEGOZIO] Libero il commesso %d dalla supervisione del cliente %d...\n", i, richiesta.id)

								if commessi[i].dentro && commessi[i].vuole_uscire && commessi[i].numero_clienti_assegnati == 0 {
									fmt.Printf("[NEGOZIO] Il commesso %d esce dal negozio...\n", i)

									commessi[i].dentro = false
									commessi[i].vuole_uscire = false
									commessi[i].ack_uscita <- true
									commessi[i].ack_uscita = nil

									for j := 0; j < 3; j++ {
										commessi[i].clienti_assegnati[j] = -1
									}

									commessi_dentro--
									commessi_liberi--
								}
							}
						}
					}
				}
			}
		case <-terminaNegozio:
			{
				fmt.Printf("[NEGOZIO] Termino...\n")
				terminaNegozio <- true
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

	for i := 0; i < N_CLIENTI; i++ {
		var tipo int

		if rand.Intn(100) > 70 {
			tipo = ABITUALI
		} else {
			tipo = OCCASIONALI
		}

		go cliente(i, tipo)
	}

	for i := 0; i < N_COMMESSI; i++ {
		terminaCommesso[i] = make(chan bool, MAXBUFF)
		go commesso(i, terminaCommesso[i], done)
	}

	go fornitore()
	go negozio()

	for i := 0; i < N_CLIENTI; i++ {
		<-termina_cliente
	}

	terminaFornitore <- true
	<-terminaFornitore

	for i := 0; i < N_COMMESSI; i++ {
		terminaCommesso[i] <- true
	}

	for i := 0; i < N_COMMESSI; i++ {
		<-done
	}

	terminaNegozio <- true
	<-terminaNegozio
}
