int main() {
    int i = 5;
    int c = 0;
    
    for (i=0;i<5;i++) {
        if (i > 3) {
            c = 2;
            break;

        }
        c++;
        
        ++c;
        
    }
    write(c);
    return 0;
}