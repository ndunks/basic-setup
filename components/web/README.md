# maxblast-dash

This template should help get you started developing with Vue 3 in Vite.

## Recommended IDE Setup

[VSCode](https://code.visualstudio.com/) + [Volar](https://marketplace.visualstudio.com/items?itemName=Vue.volar) (and disable Vetur) + [TypeScript Vue Plugin (Volar)](https://marketplace.visualstudio.com/items?itemName=Vue.vscode-typescript-vue-plugin).

## Type Support for `.vue` Imports in TS

TypeScript cannot handle type information for `.vue` imports by default, so we replace the `tsc` CLI with `vue-tsc` for type checking. In editors, we need [TypeScript Vue Plugin (Volar)](https://marketplace.visualstudio.com/items?itemName=Vue.vscode-typescript-vue-plugin) to make the TypeScript language service aware of `.vue` types.

If the standalone TypeScript plugin doesn't feel fast enough to you, Volar has also implemented a [Take Over Mode](https://github.com/johnsoncodehk/volar/discussions/471#discussioncomment-1361669) that is more performant. You can enable it by the following steps:

1. Disable the built-in TypeScript Extension
    1) Run `Extensions: Show Built-in Extensions` from VSCode's command palette
    2) Find `TypeScript and JavaScript Language Features`, right click and select `Disable (Workspace)`
2. Reload the VSCode window by running `Developer: Reload Window` from the command palette.

## Customize configuration

See [Vite Configuration Reference](https://vitejs.dev/config/).

## Project Setup

```sh
npm install
```

### Compile and Hot-Reload for Development

```sh
npm run dev
```

### Type-Check, Compile and Minify for Production

```sh
npm run build
```


## Sample Data


``` json
{
    "data": [
        {
            "name": "rewardlinkpass",
            "components": [
                {
                    "type": "BODY",
                    "text": "*{{1}}* adalah kode verifikasi Anda. Demi keamanan, jangan bagikan kode ini.",
                    "example": {
                        "body_text": [
                            [
                                "123456"
                            ]
                        ]
                    }
                },
                {
                    "type": "BUTTONS",
                    "buttons": [
                        {
                            "type": "URL",
                            "text": "Salin kode",
                            "url": "https://www.whatsapp.com/otp/code/?otp_type=COPY_CODE&code=otp{{1}}",
                            "example": [
                                "https://www.whatsapp.com/otp/code/?otp_type=COPY_CODE&code=otp123456"
                            ]
                        }
                    ]
                }
            ],
            "language": "id",
            "status": "APPROVED",
            "category": "AUTHENTICATION",
            "id": "735319491729261"
        },
        {
            "name": "rewardlinkbaf1",
            "components": [
                {
                    "type": "HEADER",
                    "format": "DOCUMENT",
                    "example": {
                        "header_handle": [
                            "https://scontent.whatsapp.net/v/t61.29466-34/354157958_1446857299463082_8465555081080462907_n.pdf?ccb=1-7&_nc_sid=57045b&_nc_ohc=rnIBIoqNTBgAX-84B_2&_nc_ht=scontent.whatsapp.net&edm=ANHSQoYEAAAA&oh=01_AdSJeF_JLZRl32xiygRV9w9ISLj2SqGLMrvEKn6BUK49pw&oe=64CF29E8"
                        ]
                    }
                },
                {
                    "type": "BODY",
                    "text": "Hi ({{1}}), Klaim Voucher-mu Sekarang!\n\nTerima kasih sudah membayar cicilan pertamamu tepat waktu! 🥰\nSelamat kamu terpilih untuk mendapatkan Voucher {{2}} senilai {{3}},- dari BAF PraDana.\n\nVoucher berlaku sampai {{4}}. Yuk, download voucher-mu sekarang! Klik link di bawah ini:\n{{5}}\n\nInformasi lengkap hubungi BAF Care 1500750\n\nMenjadi konsumen berkredibilitas keuangan yang baik dan tercatat di Otoritas Jasa Keuangan, penuhi kewajiban konsumen dengan membayar cicilan tepat waktu. \nBAF, Proses Cepat Angsuran Tepat! \nPT Bussan Auto Finance berizin dan diawasi oleh Otoritas Jasa Keuangan.",
                    "example": {
                        "body_text": [
                            [
                                "Tono",
                                "Gopay",
                                "5000",
                                "23 Desember 2023",
                                "https://tes20rbrequest.rewardplus-staging.com/tes-1000-link/5d3267a4-5a92-40bd-8f3c-6c31aef430fb"
                            ]
                        ]
                    }
                }
            ],
            "language": "id",
            "status": "APPROVED",
            "category": "MARKETING",
            "id": "1446857296129749"
        },
        {
            "name": "loyalty_alfa",
            "components": [
                {
                    "type": "HEADER",
                    "format": "TEXT",
                    "text": "Hi {{1}}",
                    "example": {
                        "header_text": [
                            "Arif"
                        ]
                    }
                },
                {
                    "type": "BODY",
                    "text": "Terima kasih untuk pembayaran {{1}}\n\nKlik link ini untuk tukar voucher anda {{2}}",
                    "example": {
                        "body_text": [
                            [
                                "Voucher Indomaret 100.000",
                                "https://indosattbk.rewardplus.xyz/indosat-po-200080252-07-desember-2022/efcea057-d707-423f-a5fd-80a82edee8d4"
                            ]
                        ]
                    }
                }
            ],
            "language": "id",
            "status": "APPROVED",
            "category": "UTILITY",
            "id": "1615316518975030"
        },
        {
            "name": "kode_otp",
            "components": [
                {
                    "type": "BODY",
                    "text": "Hi {{1}}\n\nKode anda {{2}}\n\nTerimakasih",
                    "example": {
                        "body_text": [
                            [
                                "Arif",
                                "123472"
                            ]
                        ]
                    }
                }
            ],
            "language": "id",
            "status": "REJECTED",
            "category": "AUTHENTICATION",
            "id": "236692965531518"
        },
        {
            "name": "notifa",
            "components": [
                {
                    "type": "HEADER",
                    "format": "TEXT",
                    "text": "\"Hi Bapak/Ibu, Klaim Voucher-mu Sekarang!"
                },
                {
                    "type": "BODY",
                    "text": "Terima kasih sudah membayar cicilan pertamamu tepat waktu! 🥰\nSelamat kamu terpilih untuk mendapatkan Voucher Alfamart senilai Rp.25000\n \nLink voucher dapat digunakan sebelum {{1}}\nKlik link di bawah ini: {{2}}\n\nInformasi lengkap hubungi {{3}}",
                    "example": {
                        "body_text": [
                            [
                                "11 Juli 2023",
                                "https://indosattbk.rewardplus.xyz/indosat-po-200080252-07-desember-2022/efcea057-d707-423f-a5fd-80a82edee8d4",
                                "Customer service 628111360800"
                            ]
                        ]
                    }
                },
                {
                    "type": "FOOTER",
                    "text": "Catatan: Mohon balas \"\"YA\"\" setelah menerima pesan untuk mem"
                },
                {
                    "type": "BUTTONS",
                    "buttons": [
                        {
                            "type": "QUICK_REPLY",
                            "text": "Ya"
                        }
                    ]
                }
            ],
            "language": "id",
            "status": "APPROVED",
            "category": "UTILITY",
            "id": "183311817850663"
        },
        {
            "name": "hello_world",
            "components": [
                {
                    "type": "HEADER",
                    "format": "TEXT",
                    "text": "Hello World"
                },
                {
                    "type": "BODY",
                    "text": "Welcome and congratulations!! This message demonstrates your ability to send a WhatsApp message notification from the Cloud API, hosted by Meta. Thank you for taking the time to test with us."
                },
                {
                    "type": "FOOTER",
                    "text": "WhatsApp Business Platform sample message"
                }
            ],
            "language": "en_US",
            "status": "APPROVED",
            "category": "UTILITY",
            "id": "1789167568182308"
        }
    ],
    "paging": {
        "cursors": {
            "before": "MAZDZD",
            "after": "MjQZD"
        }
    }
}
```