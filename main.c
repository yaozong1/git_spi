#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include <string.h>
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "mcp_can.h"
#include "mcp_can_dfs.h"

#include "nrf_drv_gpiote.h"
#include "nrf_gpio.h"
//#include "nrf_drv_clock.h"

#define SPI_INSTANCE  0 /**< SPI instance index. */
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */

static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */

#define TEST_STRING "Nordic"
static uint8_t       m_tx_buf[] = TEST_STRING;           /**< TX buffer. */
static uint8_t       m_rx_buf[sizeof(TEST_STRING) + 1];    /**< RX buffer. */
static const uint8_t m_length = sizeof(m_tx_buf);        /**< Transfer length. */

/**
 * @brief SPI user event handler.
 * @param event
 */

static void can_init(void)
{
    NRF_LOG_DEBUG("CAN_500K Initialization...");

    mcp_spi_init();

    NRF_LOG_DEBUG(" CAN_500K Initialized");

    START_INIT:
	
    if(mcp_can_begin(CAN_500KBPS, MCP_16MHz) == CAN_OK)
    {
	NRF_LOG_DEBUG("CAN_500K BUS Initialization ok");
    }
    else
    {
	NRF_LOG_DEBUG("CAN_500K BUS Initialization failed");
        NRF_LOG_DEBUG("Init CAN BUS again");
        nrf_delay_ms(1000);
        goto START_INIT;
    }

    NRF_LOG_DEBUG("CAN_500K Initialization COMPLETED.");
}

static void can_init2(void)
{
    NRF_LOG_DEBUG("CAN_250K Initialization...");

    mcp_spi_init2();

    NRF_LOG_DEBUG(" CAN_250K Initialized");

    START_INIT:
	
    if(mcp_can_begin(CAN_250KBPS, MCP_16MHz) == CAN_OK)
    {
	NRF_LOG_DEBUG("CAN_250K BUS Initialization ok");
    }
    else
    {
	NRF_LOG_DEBUG("CAN_250K BUS Initialization failed");
        NRF_LOG_DEBUG("Init CAN BUS again");
        nrf_delay_ms(1000);
        goto START_INIT;
    }

    NRF_LOG_DEBUG("CAN_250K Initialization COMPLETED.");
}


void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
    spi_xfer_done = true;
    NRF_LOG_INFO("Transfer completed.");
    if (m_rx_buf[0] != 0)
    {
        NRF_LOG_INFO(" Received:");
        NRF_LOG_HEXDUMP_INFO(m_rx_buf, strlen((const char *)m_rx_buf));
    }
}

int main(void)
{

   // bsp_board_init(BSP_INIT_LEDS);
    NRF_LOG_INFO("SPI example started.");
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0,1));//CAN
    nrf_gpio_pin_write(NRF_GPIO_PIN_MAP(0,1), 1);
    NRF_LOG_FLUSH();

    ret_code_t err_code;

    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    NRF_LOG_INFO("SPI example started.");


    NRF_LOG_INFO("SPI example started.");
 //   mcp_spi_init();
    can_init();
 //   nrf_drv_gpiote_in_init(MCP2515_PIN_INT, &mcp2515_int_config, mcp2515_int_pin_handler);
    nrf_drv_gpiote_in_event_enable(MCP2515_PIN_INT, false);
    nrf_drv_gpiote_in_uninit(MCP2515_PIN_INT);
    mcp_spi_uninit();
    can_init2();
    //nrf_drv_gpiote_in_event_enable(MCP2515_PIN_INT_2, false);
    //nrf_drv_gpiote_in_uninit(MCP2515_PIN_INT_2);
    //mcp_spi_uninit();
    //can_init();





    while (1)
    {
        // Reset rx buffer and transfer done flag
        memset(m_rx_buf, 0, m_length);
        spi_xfer_done = false;

        APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, m_tx_buf, m_length, m_rx_buf, m_length));

        while (!spi_xfer_done)
        {
            __WFE();
        }

        NRF_LOG_FLUSH();

        bsp_board_led_invert(BSP_BOARD_LED_0);
        nrf_delay_ms(200);
    }
}
