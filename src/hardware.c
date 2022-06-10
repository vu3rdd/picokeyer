float element_time_ms(int wpm) {
    // PARIS = 50 elements
    // x WPM means, x times 50 elements in 60 seconds.

    return 1500.0/(float)wpm; // (1000.0 * 60.0)/((float)wpm * (float)50)
}
