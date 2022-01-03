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
const ROSSO = 0
const GIALLO = 1
const VERDE = 2
const BIANCO = 3
const N = 10

var tipoPaziente = [2]string{"Adulto", "Pediatrico"}
var tipoCodice = [4]string{"Rosso", "Giallo", "Verde", "Bianco"}

/////////////////////////////////////////////////////////////////////
// Strutture Dati
/////////////////////////////////////////////////////////////////////
type Richiesta struct {
	id   int
	tipo int
	ack  chan int
}

/////////////////////////////////////////////////////////////////////
// Canali
/////////////////////////////////////////////////////////////////////
var richiesta_codice_urgenza = make(chan Richiesta, MAXBUFF)
var visita_ambulatorio_adulti [4]chan Richiesta
var visita_ambulatorio_pediatrico [4]chan Richiesta
var fine_visita_adulti = make(chan Richiesta, MAXBUFF)
var fine_visita_pediatrico = make(chan Richiesta, MAXBUFF)

/////////////////////////////////////////////////////////////////////
// Join Goroutine
/////////////////////////////////////////////////////////////////////
var done = make(chan bool)
var termina_adulti = make(chan bool)
var termina_pediatrico = make(chan bool)
var termina_triage = make(chan bool)

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
func paziente(id int, tipo int) {
	richiesta := Richiesta{id, tipo, make(chan int)}

	fmt.Printf("[Paziente %s %d]: avviato\n", tipoPaziente[tipo], id)

	richiesta_codice_urgenza <- richiesta
	codice := <-richiesta.ack

	fmt.Printf("[Paziente %s %d]: ottenuto codice di urgenza %s\n", tipoPaziente[tipo], id, tipoCodice[codice])

	sleepRandTime(3)

	if tipo == 0 {
		visita_ambulatorio_adulti[codice] <- richiesta
	} else if tipo == 1 {
		visita_ambulatorio_pediatrico[codice] <- richiesta
	}

	<-richiesta.ack
	fmt.Printf("[Paziente %s %d]: sono stato visitato con codice %s\n", tipoPaziente[tipo], id, tipoCodice[codice])

	if tipo == 0 {
		fine_visita_adulti <- richiesta
	} else if tipo == 1 {
		fine_visita_pediatrico <- richiesta
	}

	<-richiesta.ack
	fmt.Printf("[Paziente %s %d]: esco dall'ospedale\n", tipoPaziente[tipo], id)

	done <- true
	return
}

func triage() {
	fmt.Printf("[Triage]: avviato\n")

	for {
		select {
		case richiesta := <-richiesta_codice_urgenza:
			codice := rand.Intn(4)
			richiesta.ack <- codice
			fmt.Printf("[Triage]: al paziente %s numero %d è stato dato il codice %s\n", tipoPaziente[richiesta.tipo], richiesta.id, tipoCodice[codice])
		case <-termina_triage:
			fmt.Printf("[Triage]: termino\n")
			done <- true
			return
		}
	}
}

func ambulatorio_pediatrico() {
	var medici_disponibili int = N

	for {
		select {
		case richiesta := <-when(medici_disponibili > 0, visita_ambulatorio_pediatrico[ROSSO]):
			medici_disponibili--
			richiesta.ack <- 1
			fmt.Printf("[Ambulatorio P]: paziente %s %d con codice ROSSO visitato. Num medici disponibili: %d\n", tipoPaziente[richiesta.tipo], richiesta.id, medici_disponibili)
		case richiesta := <-when(medici_disponibili > 0 && len(visita_ambulatorio_pediatrico[ROSSO]) == 0, visita_ambulatorio_pediatrico[GIALLO]):
			medici_disponibili--
			richiesta.ack <- 1
			fmt.Printf("[Ambulatorio P]: paziente %s %d con codice GIALLO visitato. Num medici disponibili: %d\n", tipoPaziente[richiesta.tipo], richiesta.id, medici_disponibili)
		case richiesta := <-when(medici_disponibili > 0 && len(visita_ambulatorio_pediatrico[ROSSO]) == 0 && len(visita_ambulatorio_pediatrico[GIALLO]) == 0, visita_ambulatorio_pediatrico[VERDE]):
			medici_disponibili--
			richiesta.ack <- 1
			fmt.Printf("[Ambulatorio P]: paziente %s %d con codice VERDE visitato. Num medici disponibili: %d\n", tipoPaziente[richiesta.tipo], richiesta.id, medici_disponibili)
		case richiesta := <-when(medici_disponibili > 0 && len(visita_ambulatorio_pediatrico[ROSSO]) == 0 && len(visita_ambulatorio_pediatrico[GIALLO]) == 0 && len(visita_ambulatorio_pediatrico[VERDE]) == 0, visita_ambulatorio_pediatrico[BIANCO]):
			medici_disponibili--
			richiesta.ack <- 1
			fmt.Printf("[Ambulatorio P]: paziente %s %d con codice BIANCO visitato. Num medici disponibili: %d\n", tipoPaziente[richiesta.tipo], richiesta.id, medici_disponibili)
		case richiesta := <-fine_visita_pediatrico:
			medici_disponibili++
			richiesta.ack <- 1
			fmt.Printf("[Ambulatorio P]: paziente %s %d se ne va. Num medici disponibili: %d\n", tipoPaziente[richiesta.tipo], richiesta.id, medici_disponibili)
		case <-termina_pediatrico:
			fmt.Printf("[Ambulatorio P]: fine\n")
			done <- true
			return
		}
	}
}

func ambulatorio_adulti() {
	var medici_disponibili int = N

	for {
		select {
		case richiesta := <-when(medici_disponibili > 0, visita_ambulatorio_adulti[ROSSO]):
			medici_disponibili--
			richiesta.ack <- 1
			fmt.Printf("[Ambulatorio P]: paziente %s %d con codice ROSSO visitato. Num medici disponibili: %d\n", tipoPaziente[richiesta.tipo], richiesta.id, medici_disponibili)
		case richiesta := <-when(medici_disponibili > 0 && len(visita_ambulatorio_adulti[ROSSO]) == 0, visita_ambulatorio_adulti[GIALLO]):
			medici_disponibili--
			richiesta.ack <- 1
			fmt.Printf("[Ambulatorio P]: paziente %s %d con codice GIALLO visitato. Num medici disponibili: %d\n", tipoPaziente[richiesta.tipo], richiesta.id, medici_disponibili)
		case richiesta := <-when(medici_disponibili > 0 && len(visita_ambulatorio_adulti[ROSSO]) == 0 && len(visita_ambulatorio_adulti[GIALLO]) == 0, visita_ambulatorio_adulti[VERDE]):
			medici_disponibili--
			richiesta.ack <- 1
			fmt.Printf("[Ambulatorio P]: paziente %s %d con codice VERDE visitato. Num medici disponibili: %d\n", tipoPaziente[richiesta.tipo], richiesta.id, medici_disponibili)
		case richiesta := <-when(medici_disponibili > 0 && len(visita_ambulatorio_adulti[ROSSO]) == 0 && len(visita_ambulatorio_adulti[GIALLO]) == 0 && len(visita_ambulatorio_adulti[VERDE]) == 0, visita_ambulatorio_adulti[BIANCO]):
			medici_disponibili--
			richiesta.ack <- 1
			fmt.Printf("[Ambulatorio P]: paziente %s %d con codice BIANCO visitato. Num medici disponibili: %d\n", tipoPaziente[richiesta.tipo], richiesta.id, medici_disponibili)
		case richiesta := <-fine_visita_adulti:
			medici_disponibili++
			richiesta.ack <- 1
			fmt.Printf("[Ambulatorio P]: paziente %s %d se ne va. Num medici disponibili: %d\n", tipoPaziente[richiesta.tipo], richiesta.id, medici_disponibili)
		case <-termina_adulti:
			fmt.Printf("[Ambulatorio P]: fine\n")
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

	var num int = 50

	for i := 0; i < 4; i++ {
		visita_ambulatorio_adulti[i] = make(chan Richiesta, MAXBUFF)
		visita_ambulatorio_pediatrico[i] = make(chan Richiesta, MAXBUFF)
	}

	go triage()
	go ambulatorio_adulti()
	go ambulatorio_pediatrico()

	for i := 0; i < num; i++ {
		go paziente(i, rand.Intn(2))
	}

	// join
	for i := 0; i < num; i++ {
		<-done
	}

	termina_triage <- true
	<-done
	termina_adulti <- true
	<-done
	termina_pediatrico <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
