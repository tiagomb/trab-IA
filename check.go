package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

// nextState calcula o próximo estado da grade no Jogo da Vida
func nextState(grid [][]int) [][]int {
	rows := len(grid)
	cols := len(grid[0])
	newGrid := make([][]int, rows)
	for i := range newGrid {
		newGrid[i] = make([]int, cols)
	}

	// Definindo os 8 vizinhos possíveis
	neighbors := [][2]int{
		{-1, -1}, {-1, 0}, {-1, 1},
		{0, -1}, {0, 1},
		{1, -1}, {1, 0}, {1, 1},
	}

	for r := 0; r < rows; r++ {
		for c := 0; c < cols; c++ {
			aliveNeighbors := 0

			// Contando os vizinhos vivos
			for _, n := range neighbors {
				nr, nc := r+n[0], c+n[1]
				if nr >= 0 && nr < rows && nc >= 0 && nc < cols {
					aliveNeighbors += grid[nr][nc]
				}
			}

			// Aplicando as regras do Jogo da Vida
			if grid[r][c] == 1 {
				// A célula está viva: sobrevive se tiver 2 ou 3 vizinhos
				if aliveNeighbors == 2 || aliveNeighbors == 3 {
					newGrid[r][c] = 1
				} else {
					newGrid[r][c] = 0
				}
			} else {
				// A célula está morta: se tiver exatamente 3 vizinhos, ela nasce
				if aliveNeighbors == 3 {
					newGrid[r][c] = 1
				}
			}
		}
	}

	return newGrid
}

// checa se duas grades são iguais
func areEqual(grid1, grid2 [][]int) bool {
	for i := 0; i < len(grid1); i++ {
		for j := 0; j < len(grid1[0]); j++ {
			if grid1[i][j] != grid2[i][j] {
				return false
			}
		}
	}
	return true
}

func printGrid(grid [][]int) {
	for _, row := range grid {
		for _, cell := range row {
			fmt.Printf("%d ", cell)
		}
		fmt.Println()
	}
}

// lê a grade da entrada padrão
func readGrid() [][]int {
	scanner := bufio.NewScanner(os.Stdin)
	fmt.Println("Enter the grid row by row (0s and 1s separated by spaces). End input with an empty line:")

	var grid [][]int
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if line == "" {
			break
		}
		row := []int{}
		for _, val := range strings.Fields(line) {
			num, err := strconv.Atoi(val)
			if err != nil {
				fmt.Println("Invalid input. Please enter only 0s and 1s.")
				os.Exit(1)
			}
			row = append(row, num)
		}
		grid = append(grid, row)
	}

	if err := scanner.Err(); err != nil {
		fmt.Println("Error reading input:", err)
		os.Exit(1)
	}

	return grid
}

func main() {
	// Lê o estado atual (t0) e o estado alvo (t1)
	fmt.Println("Enter the current state (t0):")
	t0 := readGrid()

	fmt.Println("Enter the target state (t1):")
	t1 := readGrid()

	// Imprime as grades
	fmt.Println("t0:")
	printGrid(t0)

	fmt.Println("t1:")
	printGrid(t1)

	// Verifica se t1 é o próximo estado de t0
	nextT0 := nextState(t0)
	if areEqual(nextT0, t1) {
		fmt.Println("t1 is the next state of t0.")
	} else {
		fmt.Println("t1 is not the next state of t0.")
	}
}
