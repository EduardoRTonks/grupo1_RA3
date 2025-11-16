# Relatório de Experimento: Cgroup Throttling (Aluno 4)

Este relatório documenta o experimento de "throttling" (limitação de recursos)
usando a API Cgroups v2 no Linux.

## Metodologia

1.  Um programa de stress (`test_cpu`) foi compilado e executado em background.
2.  Um script de experimento (`experimento_cgroup.c`) foi usado para:
    * Criar um novo cgroup v2 chamado `teste-cpu-50`.
    * Aplicar um limite de CPU de 0.5 cores (50% de 1 core) usando o arquivo `cpu.max`.
    * Mover o PID do `test_cpu` para o arquivo `cgroup.procs` do novo cgroup.
3.  A ferramenta `resource_monitor` foi usada para validar se o limite foi aplicado.

## Resultados

A ferramenta `resource_monitor` confirmou o sucesso do experimento. Conforme a
saída abaixo, o processo (PID 188419) foi limitado a ~50% de uso de CPU,
o que foi verificado tanto pela leitura do `/proc` (linha `CPU:`) quanto
pela leitura das métricas do Cgroup (linha `Cgroup CPU:`).