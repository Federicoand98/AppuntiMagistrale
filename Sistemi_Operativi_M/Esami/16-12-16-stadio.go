package main

import (
	"fmt"
	"math/rand"
	"time"
)

const MAXBUFF = 100
const MAXPROC = 50
const NUM = 3
const LOCALI = 0
const OSPITI = 1

//definizione canali
var done = make(chan bool)
var termina_stadio = make(chan bool)
var termina_biglietteria = make(chan bool)
var ingresso_ospiti = make(chan int, MAXBUFF)
var ingresso_locali = make(chan int, MAXBUFF)
var uscita_ospiti = make(chan int)
var uscita_locali = make(chan int)
var acquisto = make(chan int, MAXBUFF)
var ACK [MAXPROC]chan int

func when(b bool, c chan int) chan int {
	if !b {
		return nil
	}
	return c
}

func spettatore(id int) {
	var biglietto, soldi int
	var entrata, uscita chan int
	var nomeTribuna string

	fmt.Printf("[ spettatore %d ] -> partito\n", id)

	time.Sleep(time.Duration(rand.Intn(5)+1) * time.Second)

	soldi = rand.Intn(10) + 1
	fmt.Printf("[ spettatore %d ] -> acquisto biglietto in biglietteria con offerta di euro %d\n", id, soldi)

	acquisto <- id
	biglietto = <-ACK[id]

	if biglietto == LOCALI {
		entrata = ingresso_locali
		uscita = uscita_locali
		nomeTribuna = "LOCALI"
	} else if biglietto == OSPITI {
		entrata = ingresso_locali
		uscita = uscita_locali
		nomeTribuna = "OSPITI"
	}

	time.Sleep(time.Duration(rand.Intn(5)+1) * time.Second)
	fmt.Printf("[ spettatore %d ] -> richiedo accesso a tribuna %s\n", id, nomeTribuna)

	entrata <- id
	<-ACK[id]

	fmt.Printf("[ spettatore %d ] -> controllo in corso...\n", id)
	time.Sleep(time.Duration(rand.Intn(5)+1) * time.Second)

	fmt.Printf("[ spettatore %d ] -> ok, entro alla mia tribuna (%s)\n", id, nomeTribuna)
	uscita <- id
	done <- true
	return
}

func biglietteria() {
	var tribuna int

	for {
		select {
		case x := <-acquisto:
			tribuna = rand.Intn(2)
			fmt.Printf("[ biglietteria ] -> spettatore %d va in tribuna %d\n", x, tribuna)
			ACK[x] <- tribuna

		case <-termina_biglietteria:
			fmt.Printf("[ tribuna ] -> ho finito\n")
			done <- true
			return
		}
	}
}

func varco() {
	var num_ospiti, num_locali int
	var operatori_occupati int

	fmt.Printf("[ varco ] -> partito\n")

	for {
		select {
		case x := <-when((operatori_occupati < NUM && ((num_locali >= num_ospiti) || (num_locali < num_ospiti && len(ingresso_ospiti) == 0))), ingresso_locali):
			operatori_occupati++
			fmt.Printf("[ varco ] -> spettatore %d inizia controlli per LOCALI\n", x)
			ACK[x] <- 1

		case x := <-when((operatori_occupati < NUM && ((num_ospiti >= num_locali) || (num_ospiti < num_locali && len(ingresso_locali) == 0))), ingresso_ospiti):
			operatori_occupati++
			fmt.Printf("[ varco ] -> spettatore %d inizia controlli per OSPITI\n", x)
			ACK[x] <- 1

		case x := <-uscita_locali:
			operatori_occupati--
			num_locali++
			fmt.Printf("[ varco ] -> spettatore %d ha superato controlli, entra in LOCALI\n", x)

		case x := <-uscita_ospiti:
			operatori_occupati--
			num_ospiti++
			fmt.Printf("[ varco ] -> spettatore %d ha superato controlli, entra in OSPITI\n", x)

		case <-termina_stadio:
			fmt.Printf("[ varco ] -> ho finito\n")
			done <- true
			return
		}
	}
}

func main() {
	var S int

	rand.Seed(time.Now().Unix())
	fmt.Printf("\nQuanti spettatori (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &S)

	for i := 0; i < S; i++ {
		ACK[i] = make(chan int, MAXBUFF)
	}

	go varco()
	go biglietteria()

	for i := 0; i < S; i++ {
		go spettatore(i)
	}

	for i := 0; i < S; i++ {
		<-done
	}

	termina_stadio <- true
	termina_biglietteria <- true
	<-done
	<-done

	fmt.Printf("\nFine.\n")
}
