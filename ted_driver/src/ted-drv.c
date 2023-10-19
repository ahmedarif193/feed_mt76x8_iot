#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

#define GPIO_START 416
#define GPIO_END 416+32+32

static int button_irqs[GPIO_END - GPIO_START + 1];

static irqreturn_t button_irq_handler(int irq, void *dev_id) {
    int gpio_num = (int)dev_id;
    if(gpio_num != 492 && gpio_num != 491)
    printk(KERN_INFO "Button with GPIO %d pressed!\n", gpio_num);
    return IRQ_HANDLED;
}

static int __init gpio_listener_init(void) {
    int i, retval;
    int gpio_num;

    printk(KERN_INFO "GPIO Listener: Initialization\n");

    for (i = GPIO_START; i <= GPIO_END; i++) {
        gpio_num = i;

        retval = gpio_request(gpio_num, "button");
        if (retval) {
            printk(KERN_ERR "Failed to request GPIO %d: error %d.\n", gpio_num, retval);
            continue;
        }

        gpio_direction_input(gpio_num);

        // Register interrupt handler for each GPIO
        button_irqs[i - GPIO_START] = gpio_to_irq(gpio_num);
        retval = request_irq(button_irqs[i - GPIO_START], button_irq_handler, IRQF_TRIGGER_FALLING, "button_irq", (void *)gpio_num);

        if (retval) {
            printk(KERN_ERR "Failed to request IRQ for GPIO %d: error %d.\n", gpio_num, retval);
            gpio_free(gpio_num);
        }
    }

    return 0;
}

static void __exit gpio_listener_exit(void) {
    int i;

    printk(KERN_INFO "GPIO Listener: Exiting\n");

    for (i = GPIO_START; i <= GPIO_END; i++) {
        free_irq(button_irqs[i - GPIO_START], (void *)i);
        gpio_free(i);
    }
}

module_init(gpio_listener_init);
module_exit(gpio_listener_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("GPIO Listener from 416 to 450");
