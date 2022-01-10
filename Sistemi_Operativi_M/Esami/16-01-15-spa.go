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
const NP = 10
const NS = 10
const CLIENTI_P = 0
const CLIENTI_S = 1
const CLIENSI_PS = 2
const OPERATORI_P = 0
const OPERATORI_S = 1

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
var ingresso_clienti [3]chan Richiesta
var ingresso_operatori [2]chan Richiesta
var uscita_clienti [3]chan Richiesta
var uscita_operatori [3]chan Richiesta

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

func printTipoCliente(tipo int) string {
	switch tipo {
	case CLIENTI_P:
		return "PISCINA"
	case CLIENTI_S:
		return "SPA"
	case CLIENSI_PS:
		return "PISCINA+SPA"
	default:
		return ""
	}
}

func printTipoOperatore(tipo int) string {
	switch tipo {
	case OPERATORI_P:
		return "PISCINA"
	case OPERATORI_S:
		return "SPA"
	default:
		return ""
	}
}

/////////////////////////////////////////////////////////////////////
// Goroutine
/////////////////////////////////////////////////////////////////////
func cliente(id int, tipo int) {
	richiesta := Richiesta{id, make(chan int)}

	ingresso_clienti[tipo] <- richiesta
	<-richiesta.ack

	fmt.Printf("[Cliente %d]: entrato in %s\n", id, printTipoCliente(tipo))

	time.Sleep(time.Duration(rand.Intn(5)+1) * time.Second)

	uscita_clienti[tipo] <- richiesta
	<-richiesta.ack

	fmt.Printf("[Cliente %d]: uscito da %s\n", id, printTipoCliente(tipo))

	done <- true
}

func operatore(id int, tipo int) {
	richiesta := Richiesta{id, make(chan int)}

	uscita_operatori[tipo] <- richiesta
	<-richiesta.ack

	fmt.Printf("[Operatore %d]: uscito da %s per una pausa\n", id, printTipoOperatore(tipo))

	time.Sleep(time.Duration(rand.Intn(5)+1) * time.Second)

	ingresso_operatori[tipo] <- richiesta
	<-richiesta.ack

	fmt.Printf("[Operatore %d]: rientrato in %s dalla pausa\n", id, printTipoOperatore(tipo))

	done <- true
}

func gestore() {
	var clienti_s int = 0
	var clienti_p int = 0
	var operatori_p int = 2
	var operatori_s int = 2

	for {
		select {
		case richiesta := <-when((clienti_s < NS && clienti_p < NP), ingresso_clienti[CLIENSI_PS]):
			clienti_p++
			clienti_s++
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: ingresso cliente PS. Numero spa: %d, Numero piscina: %d, Numero spa+piscina: %d,  Numero operatori p: %d, Numero operatori s: %d", clienti_s, clienti_p, clienti_p+clienti_s, operatori_p, operatori_s)
		case richiesta := <-when((clienti_p < NP && len(ingresso_clienti[CLIENSI_PS]) == 0), ingresso_clienti[CLIENTI_P]):
			clienti_p++
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: ingresso cliente P. Numero spa: %d, Numero piscina: %d, Numero spa+piscina: %d,  Numero operatori p: %d, Numero operatori s: %d", clienti_s, clienti_p, clienti_p+clienti_s, operatori_p, operatori_s)
		case richiesta := <-when((clienti_s < NS && len(ingresso_clienti[CLIENSI_PS]) == 0 && len(ingresso_clienti[CLIENTI_P]) == 0), ingresso_clienti[CLIENTI_S]):
			clienti_s++
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: ingresso cliente S. Numero spa: %d, Numero piscina: %d, Numero spa+piscina: %d,  Numero operatori p: %d, Numero operatori s: %d", clienti_s, clienti_p, clienti_p+clienti_s, operatori_p, operatori_s)
		case richiesta := <-when((clienti_p > 0 && operatori_p > 0 || clienti_p == 0), uscita_operatori[OPERATORI_P]):
			operatori_p--
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: uscita operatore P. Numero spa: %d, Numero piscina: %d, Numero spa+piscina: %d,  Numero operatori p: %d, Numero operatori s: %d", clienti_s, clienti_p, clienti_p+clienti_s, operatori_p, operatori_s)
		case richiesta := <-when((clienti_s > 0 && operatori_s > 0 || clienti_s == 0), uscita_operatori[OPERATORI_S]):
			operatori_s--
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: uscita operatore S. Numero spa: %d, Numero piscina: %d, Numero spa+piscina: %d,  Numero operatori p: %d, Numero operatori s: %d", clienti_s, clienti_p, clienti_p+clienti_s, operatori_p, operatori_s)
		case richiesta := <-uscita_clienti[CLIENSI_PS]:
			clienti_p--
			clienti_s--
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: uscita cliente PS. Numero spa: %d, Numero piscina: %d, Numero spa+piscina: %d,  Numero operatori p: %d, Numero operatori s: %d", clienti_s, clienti_p, clienti_p+clienti_s, operatori_p, operatori_s)
		case richiesta := <-uscita_clienti[CLIENTI_P]:
			clienti_p--
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: uscita cliente PS. Numero spa: %d, Numero piscina: %d, Numero spa+piscina: %d,  Numero operatori p: %d, Numero operatori s: %d", clienti_s, clienti_p, clienti_p+clienti_s, operatori_p, operatori_s)
		case richiesta := <-uscita_clienti[CLIENTI_S]:
			clienti_s--
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: uscita cliente PS. Numero spa: %d, Numero piscina: %d, Numero spa+piscina: %d,  Numero operatori p: %d, Numero operatori s: %d", clienti_s, clienti_p, clienti_p+clienti_s, operatori_p, operatori_s)
		case richiesta := <-ingresso_operatori[OPERATORI_P]:
			operatori_p++
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: ingresso operatore P. Numero spa: %d, Numero piscina: %d, Numero spa+piscina: %d,  Numero operatori p: %d, Numero operatori s: %d", clienti_s, clienti_p, clienti_p+clienti_s, operatori_p, operatori_s)
		case richiesta := <-ingresso_operatori[OPERATORI_S]:
			operatori_s++
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: ingresso operatore S. Numero spa: %d, Numero piscina: %d, Numero spa+piscina: %d,  Numero operatori p: %d, Numero operatori s: %d", clienti_s, clienti_p, clienti_p+clienti_s, operatori_p, operatori_s)
		case <-termina:
			fmt.Println("\n[Gestore]: Fine!")
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

	for i := 0; i < 3; i++ {
		ingresso_clienti[i] = make(chan Richiesta, MAXBUFF)
		uscita_clienti[i] = make(chan Richiesta, MAXBUFF)
	}

	for i := 0; i < 2; i++ {
		ingresso_operatori[i] = make(chan Richiesta, MAXBUFF)
		uscita_operatori[i] = make(chan Richiesta, MAXBUFF)
	}

	for i := 0; i < 20; i++ {
		go cliente(i, rand.Intn(2))
	}

	go operatore(1, 0)
	go operatore(2, 0)
	go operatore(3, 1)
	go operatore(4, 1)

	go gestore()

	for i := 0; i < 25; i++ {
		<-done
	}

	termina <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
