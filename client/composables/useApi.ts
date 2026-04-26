export const useApi = () => {
  const { token } = useAuth()

  async function request<T>(path: string, options: RequestInit = {}): Promise<T> {
    const headers: Record<string, string> = {
      'Content-Type': 'application/json',
      ...(options.headers as Record<string, string> || {})
    }
    if (token.value) headers['Authorization'] = `Bearer ${token.value}`

    const res = await fetch(path, { ...options, headers })
    const json = await res.json()
    if (!res.ok || json.status === 'error') {
      throw new Error(json.message || `HTTP ${res.status}`)
    }
    return json
  }

  const get  = <T>(path: string)                  => request<T>(path)
  const post = <T>(path: string, body: unknown)    => request<T>(path, { method: 'POST',   body: JSON.stringify(body) })
  const put  = <T>(path: string, body: unknown)    => request<T>(path, { method: 'PUT',    body: JSON.stringify(body) })
  const del  = <T>(path: string, body?: unknown)    => request<T>(path, { method: 'DELETE', ...(body ? { body: JSON.stringify(body) } : {}) })

  return { get, post, put, del }
}
