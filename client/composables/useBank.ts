import type { Conto, Transazione, ApiResponse } from '~/types/bank'

export const useBank = () => {
  const { get, post, del } = useApi()
  const conti = useState<Conto[]>('bank.conti', () => [])
  const transazioni = useState<Transazione[]>('bank.transazioni', () => [])

  const saldoTotale = computed(() =>
    conti.value.reduce((sum, c) => sum + c.saldo, 0)
  )

  const contoPrimario = computed(() => conti.value[0] ?? null)

  async function caricaConti() {
    const r = await get<ApiResponse<Conto[]>>('/api/conti')
    conti.value = r.data ?? []
  }

  async function caricaTransazioni(iban: string) {
    const r = await get<any>(`/api/conti/${iban}/estratto`)
    transazioni.value = r.data?.transazioni ?? []
  }

  async function preleva(importo: number, descrizione = '') {
    const r = await post<any>('/api/transazioni/preleva', { importo, descrizione })
    if (contoPrimario.value) contoPrimario.value.saldo = r.data?.saldo ?? contoPrimario.value.saldo
    await caricaConti()
  }

  async function invia(iban_destinatario: string, importo: number, descrizione = '') {
    const r = await post<any>('/api/transazioni/invia', { iban_destinatario, importo, descrizione })
    if (contoPrimario.value) contoPrimario.value.saldo = r.data?.saldo ?? contoPrimario.value.saldo
    await caricaConti()
    if (contoPrimario.value) await caricaTransazioni(contoPrimario.value.iban)
  }

  return {
    conti, transazioni, saldoTotale, contoPrimario,
    caricaConti, caricaTransazioni, preleva, invia
  }
}
