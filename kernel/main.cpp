extern "C" void KernelMain(){ //extern "C" means that we can define function in C language.
    while(1) __asm__("hlt"); //inline assemblar inorder to embed assembly language into C language.
}
//"hlt means we want to stop CPU"